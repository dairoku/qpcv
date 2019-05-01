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

#include <QtWidgets/QMainWindow>
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
        /*z = 255.0 * z;
        if (z > 255)
          z = 255;
        if (z < 0)
          z = 0;*/
        mData[width * i + j].position[0] = x;
        mData[width * i + j].position[1] = y;
        mData[width * i + j].position[2] = z;
        //
        mData[width * i + j].color[0] = 1.0;
        mData[width * i + j].color[1] = 1.0;
        mData[width * i + j].color[2] = 1.0;
      }
    mGLView->setDataPtr((float *)mData, width * height);

    ui.setupUi(this);
    setCentralWidget(mGLView);
  }

private:
  Ui::qpcvClass ui;

protected:
  // structs -------------------------------------------------------------------
  // ---------------------------------------------------------------------------
  // vertex_info
  // ---------------------------------------------------------------------------
  typedef struct
  {
    GLfloat position[3];
    GLfloat color[3];
  } PointCloudData;

  // Member variables ----------------------------------------------------------
  ibc::qt::GLPointCloudView *mGLView;
  PointCloudData *mData;

private slots:
  // ---------------------------------------------------------------------------
  // on_actionOpen_triggered
  // ---------------------------------------------------------------------------
  void on_actionOpen_triggered(void)
  {
  /*  QString fileName = QFileDialog::getOpenFileName(this,
      tr("Open RAW file"),
      "",
      tr("RAW File (*.raw);;All Files (*)"));*/
  
    QDialog dialog(this);
    Ui_OpenDialog openDialogUI;
    openDialogUI.setupUi(&dialog);
  
    if (dialog.exec() == 0)
      return;
  
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
