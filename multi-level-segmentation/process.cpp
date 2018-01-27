// multi-level-segmentation
//
// Copyright (C) 2017 Daniele Liciotti
//
// Authors: Daniele Liciotti <danielelic@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see: http://www.gnu.org/licenses/gpl-3.0.txt

#include "process.h"

Process::Process(QObject *parent) :
    QObject(parent)
{
}

Process::~Process()
{

}

void Process::init(std::string imgPath)
{
    this->depthImage = cv::Mat::zeros(240, 320, CV_16UC1);
    this->depthImage = cv::imread(imgPath, CV_LOAD_IMAGE_ANYDEPTH);
    this->imgPath = imgPath;
}

void Process::mls()
{
    for (auto i=0; i<this->depthImage.rows; ++i) {
        for (auto j=0; j<this->depthImage.cols; ++j) {
            if (this->depthImage.at<unsigned short>(cv::Point(j,i))) {
                this->depthImage.at<unsigned short>(cv::Point(j,i)) =
                        (this->depthImage.at<unsigned short>(cv::Point(j,i)) -3500)*-1;
            }
        }
    }
    // find global max point
    double maxHeight;
    cv::Point p_maxHeight;
    cv::minMaxLoc(this->depthImage, NULL, &maxHeight, NULL, &p_maxHeight);
    cv::Mat drawing;
    // begin multi-level segmentation
    std::vector<std::vector<cv::Point> > contoursVector;
    int thresholdLevel = 40;
    int minHeight = 300;

    for (auto level = maxHeight - thresholdLevel;
         level > minHeight;
         level -= thresholdLevel) {
        // find all points over the level
        cv::Mat thresholdLevelMat = this->depthImage > level;
        std::vector<std::vector<cv::Point> > contoursTemp;
        /// find contours of this level
        cv::findContours(thresholdLevelMat,
                         contoursTemp,
                         CV_RETR_EXTERNAL,
                         CV_CHAIN_APPROX_SIMPLE);
        for (auto i = 0; i < contoursTemp.size(); ++i) {
            auto area = cv::contourArea(contoursTemp[i]);
            if (area < (60)) {
                contoursTemp.erase(contoursTemp.begin() + i--);
            }
        }

        for (auto k = 0; k < contoursTemp.size(); ++k) {
            cv::Mat maskcont = cv::Mat::zeros(this->depthImage.rows,
                                              this->depthImage.cols,
                                              CV_8UC1);
            cv::Mat depthMatFilterCont = cv::Mat::zeros(this->depthImage.rows,
                                                        this->depthImage.cols,
                                                        CV_16UC1);
            cv::drawContours(maskcont, contoursTemp, k, 255, -1);
            this->depthImage.copyTo(depthMatFilterCont, maskcont);

            /// find max Point
            double maxblobValue;
            cv::Point maxblobPoint;
            cv::minMaxLoc(depthMatFilterCont,
                          NULL,
                          &maxblobValue,
                          NULL,
                          &maxblobPoint);

            auto pointCheck = false;
            for (auto m = 0; m < pheadsVec.size(); ++m) {
                if (maxblobPoint == pheadsVec[m]) {
                    pointCheck = true;
                }
            }
            if (!pointCheck) {
                pheadsVec.push_back(maxblobPoint);
                contoursVector.push_back(contoursTemp[k]);
            }
        }
    }
    std::cout << contoursVector.size() << std::endl;
    cv::Mat drawingMat = cv::Mat::zeros(this->depthImage.rows, this->depthImage.cols, CV_8UC1);
    for (auto i = 0; i < contoursVector.size(); ++i) {
        cv::drawContours(drawingMat, contoursVector, i, 255, -1, 8);
    }
    //cv::imwrite(imgPath.replace(imgPath.size()-4, imgPath.size()-1, "mls_seg.png"), drawingMat);
}
