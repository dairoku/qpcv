// =============================================================================
//  qpcv.h
//
//  MIT License
//
//  Copyright (c) 2019 Dairoku Sekiguchi
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
// =============================================================================
/*!
  \file     qpcv.h
  \author   Dairoku Sekiguchi
  \version  1.0.0
  \date     2019/05/01
  \brief    
*/

#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <QtWidgets/QMainWindow>
#include <QFileDialog>
#include "ui_qpcv.h"
#include "ui_open_dialog.h"
#include "ibc/qt/gl_point_cloud_view.h"

// -----------------------------------------------------------------------------
// qpcvWindow class
// -----------------------------------------------------------------------------
class qpcvWindow : public QMainWindow
{
Q_OBJECT

public:
  // Constructors and Destructor -----------------------------------------------
  // ---------------------------------------------------------------------------
  // qpcvWindow
  // ---------------------------------------------------------------------------
  qpcvWindow(QWidget *parent = Q_NULLPTR)
  : QMainWindow(parent)
  {
    mGLView = new ibc::qt::GLPointCloudView();
  
    size_t width = 640;
    size_t height = 480;
    //
    mData = new PointCloudData[width * height];
    //
    double xPitch = 1.0 * 2.0 / (double )width;
    for (unsigned int i = 0; i < height; i++)
      for (unsigned int j = 0; j < width; j++)
      {
        double x = -1.0 + xPitch * j;
        double y = -1.0 + xPitch * i;
        double k = (M_PI * 3.0) * (M_PI * 3.0);
        double z, d;
        if (x == 0 && y == 0)
          z = 1;
        else
        {
          d = sqrt(k*x*x + k*y*y);
          z = sin(d) / d;
        }
        mData[width * i + j].x = x;
        mData[width * i + j].y = y;
        mData[width * i + j].z = z;
        //
        mData[width * i + j].r = z;
        mData[width * i + j].g = z;
        mData[width * i + j].b = z;
      }
    mGLView->setDataPtr((float *)mData, width * height);
    //readPLY("/home/dairoku/Documents/PLY/Lucid/Shipping-Boxes.ply");

    ui.setupUi(this);
    setCentralWidget(mGLView);
  }

private:
  Ui::qpcvClass ui;

protected:
  // structs -------------------------------------------------------------------
  // ---------------------------------------------------------------------------
  // PointCloudData
  // ---------------------------------------------------------------------------
  typedef struct
  {
    GLfloat x, y, z;
    GLfloat r, g, b;
  } PointCloudData;

  // Member variables ----------------------------------------------------------
  ibc::qt::GLPointCloudView *mGLView;
  PointCloudData *mData;

  // Member functions ----------------------------------------------------------
  bool  readPLY(const char *inFileName)
  {
    printf("File : %s\n", inFileName);
    int fd = ::open(inFileName, O_RDONLY);
    if (fd == -1)
    {
      printf("Failed : open()\n");
      return false;
    }
    FILE  *fp = ::fdopen(fd, "rb");
    if (fp == NULL)
    {
      printf("Failed : fdopen()\n");
      return false;
    }
    struct stat stbuf;
    if (::fstat(fd, &stbuf) == -1)
    {
      printf("Failed : fstat()\n");
      return false;
    }
    size_t  fileSize = stbuf.st_size;
    
    unsigned char *buf = new unsigned char[fileSize];
    if (buf == NULL)
    {
      printf("Failed : buf == NULL\n");
      ::fclose(fp);
      return false;
    }

    if (::fread(buf, sizeof(unsigned char), fileSize, fp) != fileSize)
    {
      printf("Failed : fread()\n");
      ::fclose(fp);
      return false;
    }
    ::fclose(fp);

    char  *endPtr = strstr((char *)buf, "end_header");
    if (endPtr == NULL)
    {
      printf("Can't find a PLY header\n");
      return false;
    }
    endPtr += strlen("end_header");
    *endPtr = 0;
    printf("%s\n", buf);  // dump header

    size_t  headerSize = (size_t)endPtr - (size_t)buf + 1;
    size_t  dataSize = fileSize - headerSize;
    size_t  elementSize = sizeof(float) * 6 + sizeof(unsigned char) * 4;
    size_t  dataNum = dataSize / elementSize;
    printf("headerSize : %zu\n", headerSize);
    printf("dataSize : %zu\n", dataSize);
    printf("dataNum  : %zu\n", dataNum);

    memmove(buf, &(buf[headerSize]), dataSize);

    struct PLYData
    {
      float x, y, z;
      float nx, ny, nz;
      unsigned char r, g, b, a;
    };
    struct PLYData  *dataPtr = (struct PLYData *)buf;

    float x_min, y_min, z_min;
    float x_max, y_max, z_max;
    int r_min, g_min, b_min;
    int r_max, g_max, b_max;
    x_min = dataPtr->x;
    y_min = dataPtr->y;
    z_min = dataPtr->z;
    x_max = dataPtr->x;
    y_max = dataPtr->y;
    z_max = dataPtr->z;
    r_min = dataPtr->r;
    r_max = dataPtr->r;
    g_min = dataPtr->g;
    g_max = dataPtr->g;
    b_min = dataPtr->b;
    b_max = dataPtr->b;
    for (size_t i = 0; i < dataNum; i++)
    {
      if (dataPtr[i].x < x_min)
        x_min = dataPtr[i].x;
      if (dataPtr[i].y < y_min)
        y_min = dataPtr[i].y;
      if (dataPtr[i].z < z_min)
        z_min = dataPtr[i].z;
      //
      if (dataPtr[i].x > x_max)
        x_max = dataPtr[i].x;
      if (dataPtr[i].y > y_max)
        y_max = dataPtr[i].y;
      if (dataPtr[i].z > z_max)
        z_max = dataPtr[i].z;
      //
      if (dataPtr[i].r < r_min)
        r_min = dataPtr[i].r;
      if (dataPtr[i].g < g_min)
        g_min = dataPtr[i].g;
      if (dataPtr[i].b < b_min)
        b_min = dataPtr[i].b;
      //
      if (dataPtr[i].r > r_max)
        r_max = dataPtr[i].r;
      if (dataPtr[i].g > g_max)
        g_max = dataPtr[i].g;
      if (dataPtr[i].b > b_max)
        b_max = dataPtr[i].b;
    }
    printf("x: %f, %f\n", x_min, x_max);
    printf("y: %f, %f\n", y_min, y_max);
    printf("z: %f, %f\n", z_min, z_max);
    printf("r: %d, %d\n", r_min, r_max);
    printf("g: %d, %d\n", g_min, g_max);
    printf("b: %d, %d\n", b_min, b_max);
    float size_x = x_max - x_min;
    float size_y = y_max - y_min;
    float size_z = z_max - z_min;
    float size_max = size_x;
    if (size_y > size_max)
      size_max = size_y;
    if (size_z > size_max)
      size_max = size_z;
    float k = 2.0 / size_max;
    float offset_x = -1.0 * (x_min + size_x / 2.0);
    float offset_y = -1.0 * (y_min + size_y / 2.0);
    float offset_z = -1.0 * (z_min + size_z / 2.0);
    float i_gain = 1.0 / (r_max - r_min);
    float offset_i = -1.0 * r_min;

    printf("k: %f\n", k);
    printf("offset_x: %f\n", offset_x);
    printf("offset_y: %f\n", offset_y);
    printf("offset_z: %f\n", offset_z);
    printf("i_gain: %f\n", i_gain);
    printf("offset_i: %f\n", offset_i);
    //
    for (size_t i = 0; i < dataNum; i++)
    {
      mData[i].x = (dataPtr[i].x + offset_x) * k;
      mData[i].y = (dataPtr[i].y + offset_y) * k;
      mData[i].z = (dataPtr[i].z + offset_z) * k;
      mData[i].r = (dataPtr[i].r + offset_i) * i_gain;
      mData[i].g = (dataPtr[i].g + offset_i) * i_gain;
      mData[i].b = (dataPtr[i].b + offset_i) * i_gain;
    }

    mGLView->setDataPtr((float *)mData, dataNum);
    return true;
  }


private slots:
  // ---------------------------------------------------------------------------
  // on_actionOpen_triggered
  // ---------------------------------------------------------------------------
  void on_actionOpen_triggered(void)
  {
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open PLY file"),
    "",
    tr("PLY File (*.ply);;All Files (*)")); 

    readPLY(fileName.toStdString().c_str());

    /*QDialog dialog(this);
    Ui_OpenDialog openDialogUI;
    openDialogUI.setupUi(&dialog);
  
    if (dialog.exec() == 0)
      return;*/
  
    //printf("height: %s\n", openDialogUI.lineEdit_height->text().toStdString().c_str());
  }
  // ---------------------------------------------------------------------------
  // on_actionQuit_triggered
  // ---------------------------------------------------------------------------
  void on_actionQuit_triggered(void)
  {
    close();
  }
};
