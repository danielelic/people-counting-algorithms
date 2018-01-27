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

#ifndef PROCESS_H
#define PROCESS_H

#include <QtCore>
#include <QObject>

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class Process : public QObject
{
    Q_OBJECT
public:
    explicit Process(QObject *parent = 0);
    ~Process();
    void init(std::string);
    void mls();
private:
    cv::Mat depthImage;
    cv::Mat output;
    std::string imgPath;
    std::vector<cv::Point> pheadsVec;
    std::vector<std::vector<cv::Point> > contours;
};

#endif // PROCESS_H
