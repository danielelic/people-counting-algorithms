// waterfilling
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

void Process::start()
{
    this->output = waterfilling(this->depthImage, 10);

    cv::Mat segImage;
    cv::inRange(this->output, 2600, 65535, segImage);

    // find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(segImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    int nBlobs = 0;
    for (std::vector<std::vector<cv::Point> >::iterator c_iter = contours.begin(); c_iter != contours.end(); c_iter++) {
        double area = cv::contourArea(*c_iter);
        if (area > 100) {
            nBlobs++;
        } else {
            contours.erase(c_iter--);
        }
    }
    cv::Mat drawingMat = cv::Mat::zeros(depthImage.rows, depthImage.cols, CV_8UC1);
    for (auto i = 0; i < contours.size(); ++i) {
        cv::drawContours(drawingMat, contours, i, 255, -1, 8);
    }
    std::cout << nBlobs << std::endl;

    //cv::imwrite(imgPath.replace(imgPath.size()-4, imgPath.size()-1, "_wf.png"), output);
    //cv::imwrite(imgPath.replace(imgPath.size()-4, imgPath.size()-1, "_seg.png"), drawingMat);
}

// https://github.com/XucongZhang/water_filling
cv::Mat Process::waterfilling(cv::Mat InputImg, int num_of_drop)
{
    typedef ushort imgType;
    imgType maxValue = 65535; //The max possible value of every pixel, it is 255 for 8bit and 65535 for 16bit

    imgType* m_a = NULL; // pointer
    imgType* m_b = NULL;
    imgType* m_c = NULL;
    int LocX; // current location
    int LocY;

    int minP = maxValue; // to save the value of current location
    int picWidth = InputImg.cols; // picture width
    int picHeight = InputImg.rows; // picture height

    cv::Mat outputImg(picHeight, picWidth, InputImg.type()); // initialize the output image
    memset(outputImg.data, 0, picWidth * picHeight*sizeof(imgType)); // set the output image to zeros

    uchar*  Label_P = new uchar[picWidth * picHeight]; // to record whether the water had flow the position, it is a map. The value of the position is 1 if the water have been there
    memset(Label_P, 0, picWidth * picHeight); // to be zeros
    uchar* m_d = NULL; //initialization

    cv::Mat inputcopy(picHeight,picWidth, InputImg.type()); //copy the input image, since the process will infulence the original image
    InputImg.copyTo(inputcopy);
    m_a = (imgType*)inputcopy.data; // pointer to the copy image
    m_b = (imgType*)inputcopy.data; // a copy pointer to the copy image

    int index = 0;

    /// all the noise and background pixels are set to be the maxValue and no more opreation
    for (int i=0; i<picWidth*picHeight; ++i, ++m_a)
    {
        if (*m_a < 2) // before here, the noisy and background have been set to low than 2
        {
            *m_a = maxValue;
        }
    }

    // give the pointer
    m_a = (imgType*)inputcopy.data;

    int waterWay = 0;
    bool isNext = false;//if drop the water to next pixel

    //////////////////////////////////////////////////////////////
    for (int j=0; j<picHeight; j+=5, m_a+=picWidth*5) //scan the whole image with 5*5 box
    { // it means we take every 5*5 pixels to be a unit, it makes the program faster
        for (int i=0; i<picWidth; i+=5)
        {
            if (m_a[i] != maxValue) // just operate the foreground
            {
                /// initialization
                LocX = i; // the location
                LocY = j;

                m_b = (imgType*)inputcopy.data; // input image pointer
                m_b += picWidth*LocY + LocX; // move the pointer to current location
                index = 0; // I forget what this variable use for, it seems that it is useless
                index += picWidth*LocY + LocX;
                m_c = (imgType*)outputImg.data;
                m_c += picWidth*LocY + LocX; // move the pointer to current location
                m_d = Label_P;
                m_d += picWidth*LocY + LocX; // move the pointer to current location

                for (int k=0; k<25; ++k) // there are 25 waters since it represent 5*5 pixels
                {
                    if (LocX>picWidth-1 || LocX<2 || LocY>picHeight-1 || LocY<2) // do noting if on the boundary
                    {
                        break;
                    }

                    memset(Label_P, 0, picWidth * picHeight); // set to be zeros, it means every time there are no water flowed any position
                    isNext = false;//whether move to next pixel to drop water(it means the last water has been drop already)
                    // memset(Label_P, 0, picWidth * picHeight*sizeof(imgType));

                    // first compare the nearset 8 point

                    minP = *m_b; // get the central piexl value
                    waterWay = 9; // record the direction to move, 9 means the central point

                    if (minP >= *(m_b-picWidth-1)) // compare the top left
                    {
                        waterWay = 1; // 1 means the top left
                        minP =  *(m_b-picWidth-1);
                    }
                    if (minP >= *(m_b-picWidth))//top
                    {
                        waterWay = 2; // 2 means top
                        minP = *(m_b-picWidth);
                    }
                    if (minP >= *(m_b-picWidth+1)) // top right
                    {
                        waterWay = 3;
                        minP = *(m_b-picWidth+1);
                    }
                    if (minP >= *(m_b+1)) // right
                    {
                        waterWay = 4;
                        minP = *(m_b+1);
                    }
                    if (minP >= *(m_b+picWidth+1)) // bottom right
                    {
                        waterWay = 5;
                        minP = *(m_b+picWidth+1);
                    }
                    if (minP >= *(m_b+picWidth)) // bottom
                    {
                        waterWay = 6;
                        minP = *(m_b+picWidth);
                    }
                    if (minP >= *(m_b+picWidth-1)) // left bottom
                    {
                        waterWay = 7;
                        minP = *(m_b+picWidth-1);
                    }
                    if (minP >= *(m_b-1)) // left
                    {
                        waterWay = 8;
                        minP =  *(m_b-1);
                    }

                    while(!isNext) // if the water haven't been droped
                    {
                        switch(waterWay) // which way to go?
                        {
                        case 1: // top left
                            LocX -= 1; // move to the location
                            LocY -= 1;
                            if (LocX<2 || LocY <2)
                            {
                                isNext = true;
                                break; // if on the boundary, just break
                            }

                            m_b = m_b-picWidth-1; // move the top left
                            index = index-picWidth-1; // I don't know what this variable means
                            m_c = m_c-picWidth-1; // this pointer represents the output image
                            m_d = m_d-picWidth-1; // this pointer represents if the water has been flowed here before
                            *m_d = 1; // now the water has been here

                            minP = *m_b; // get the top left point value
                            waterWay = 9; // just set the direction to 9, for next loop

                            // then compare to nearest position to decide which way to move next step
                            // attention here: the water will follow so called inertia. for example, now last step the water is move to left top, so on the compare,
                            // we just compare 5 direction: top left, left, bottom left, top, and top right. And the top left has highest priority to be choosed, left and top have
                            // second level priority, and the bottom left and top right have the last priority.
                            // You can draw a picture to show these directions and you will see.
                            if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1))) // bottom left, just compare if it is low than this position and wheter the water has been here
                            {
                                waterWay = 7;
                                minP = *(m_b+picWidth-1);
                            }
                            if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1))) // top right
                            {
                                waterWay = 3;
                                minP = *(m_b-picWidth+1);
                            }
                            if (minP >= *(m_b-1) && !(*(m_d-1)))//left
                            {
                                waterWay = 8;
                                minP = *(m_b-1);
                            }
                            if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//top
                            {
                                waterWay = 2;
                                minP = *(m_b-picWidth);
                            }
                            if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))// top left, the last one has the highest priority
                            {
                                minP = *(m_b-picWidth-1);
                                waterWay = 1;
                            }
                            break;
                        case 2: // top direction, just like above.
                            LocY -= 1;
                            if (LocY < 2)
                            {
                                isNext = true;
                                break;
                            }
                            m_b -= picWidth;
                            index -= picWidth;
                            m_c -= picWidth;
                            m_d -= picWidth;
                            *m_d = 1;

                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))
                            {
                                waterWay = 1;
                                minP = *(m_b-picWidth-1);
                            }
                            if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))
                            {
                                waterWay = 3;
                                minP = *(m_b-picWidth+1);
                            }
                            if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))
                            {
                                minP = *(m_b-picWidth);
                                waterWay = 2;
                            }
                            break;
                        case 3:
                            LocY -= 1;
                            LocX += 1;
                            if (LocY<2 || LocX>picWidth-1)
                            {
                                isNext = true;break;
                            }
                            m_b = m_b-picWidth+1;
                            index = index -picWidth+1;
                            m_c = m_c-picWidth+1;
                            m_d = m_d-picWidth+1;
                            *m_d = 1;

                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))
                            {
                                waterWay = 1;
                                minP = *(m_b-picWidth-1);
                            }
                            if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)))
                            {
                                waterWay = 5;
                                minP = *(m_b+picWidth+1);
                            }
                            if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))
                            {
                                waterWay = 2;
                                minP = *(m_b-picWidth);
                            }
                            if (minP >= *(m_b+1) && !(*(m_d+1)))
                            {
                                waterWay = 4;
                                minP = *(m_b+1);
                            }
                            if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))
                            {
                                minP = *(m_b-picWidth+1);
                                waterWay = 3;
                            }
                            break;
                        case 4:
                            LocX += 1;
                            if (LocX >picWidth-1)
                            {
                                isNext = true;break;
                            }
                            m_b += 1;
                            index +=1;
                            m_c += 1;
                            m_d += 1;
                            *m_d = 1;

                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth)))
                            {
                                waterWay = 3;
                                minP = *(m_b-picWidth+1);
                            }
                            if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)))
                            {
                                waterWay = 5;
                                minP = *(m_b+picWidth+1);
                            }
                            if (minP >= *(m_b+1) && !(*(m_d+1)))
                            {
                                minP = *(m_b+1);
                                waterWay = 4;
                            }
                            break;
                        case 5:
                            LocX += 1;
                            LocY += 1;
                            if (LocX >picWidth-1 || LocY > picHeight-1)
                            {
                                isNext = true;
                                break;
                            }
                            m_b += picWidth+1;
                            index += picWidth+1;
                            m_c += picWidth+1;
                            m_d += picWidth+1;
                            *m_d = 1;
                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)) )
                            {
                                waterWay = 3;
                                minP = *(m_b-picWidth+1);
                            }
                            if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
                            {
                                waterWay = 7;
                                minP = *(m_b+picWidth-1);
                            }
                            if (minP >= *(m_b+1) && !(*(m_d+1)))
                            {
                                waterWay = 4;
                                minP =  *(m_b+1);
                            }
                            if (minP >= *(m_b+picWidth) && !(*(m_d+picWidth)) )
                            {
                                waterWay = 6;
                                minP = *(m_b+picWidth);
                            }
                            if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
                            {
                                minP = *(m_b+picWidth+1);
                                waterWay = 5;
                            }
                            break;
                        case 6:
                            LocY += 1;
                            if (LocY > picHeight-1)
                            {
                                isNext = true;
                                break;
                            }
                            m_b += picWidth;
                            index += picWidth;
                            m_c += picWidth;
                            m_d += picWidth;
                            *m_d = 1;

                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
                            {
                                waterWay = 5;
                                minP = *(m_b+picWidth+1);
                            }
                            if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
                            {
                                waterWay = 7;
                                minP = *(m_b+picWidth-1);
                            }
                            if (minP >= *(m_b+picWidth) && !(*(m_d+picWidth)) )
                            {
                                minP = *(m_b+picWidth);
                                waterWay = 6;
                            }
                            break;
                        case 7:
                            LocY += 1;
                            LocX -= 1;
                            if (LocX <2 || LocY >picHeight-1)
                            {
                                isNext = true;break;
                            }
                            m_b += picWidth-1;
                            index += picWidth-1;
                            m_c += picWidth-1;
                            m_d += picWidth-1;
                            *m_d = 1;
                            minP = *m_b;
                            waterWay = 9;

                            if (minP > *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
                            {
                                waterWay = 5;
                                minP = *(m_b+picWidth+1);
                            }
                            if (minP > *(m_b-picWidth-1) && !(*(m_d-picWidth-1)) )
                            {
                                waterWay = 1;
                                minP = *(m_b-picWidth-1);
                            }
                            if (minP > *(m_b+picWidth) && !(*(m_d+picWidth)) )
                            {
                                waterWay = 6;minP = *(m_b+picWidth);
                            }
                            if (minP > *(m_b-1) && !(*(m_d-1)) )
                            {
                                waterWay = 8;minP = *(m_b-1);
                            }
                            if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
                            {
                                minP = *(m_b+picWidth-1);
                                waterWay = 7;
                            }
                            break;
                        case 8:
                            LocX -= 1;
                            if (LocX < 2)
                            {
                                isNext = true;
                                break;
                            }

                            m_b -= 1;
                            index -=1;
                            m_c -= 1;
                            m_d -= 1;
                            *m_d = 1;

                            minP = *m_b;
                            waterWay = 9;

                            if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
                            {
                                waterWay = 7;
                                minP = *(m_b+picWidth-1);
                            }
                            if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)) )
                            {
                                waterWay = 1;
                                minP = *(m_b-picWidth-1);
                            }
                            if (minP >= *(m_b-1) && !(*(m_d-1)) )
                            {
                                minP = *(m_b-1);
                                waterWay = 8;
                            }
                            break;
                        case 9: // if it is the central point, it means that there is no way to move, so drop the water
                            *m_b += num_of_drop; // the position of input image get some water
                            *m_c += 4000; // output image also get a value, it is huge becuase it is 16bits image. The vaule is arbitrary, you can also set it to be 1, but it will hard to be seen in 16bits image
                            isNext = true; // drop the water, so go to next position
                            break;
                        }
                    }
                }
            }
        }
    }

    delete []Label_P;

    return outputImg;
}
