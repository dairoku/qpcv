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

#ifndef QPCV_WINDOW_H_
#define QPCV_WINDOW_H_

// Macros (pre) ----------------------------------------------------------------
//#define IBC_GL_FILE_PLY_TRACE_ENABLE

// Includes --------------------------------------------------------------------
#include <iostream>
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
// ibc related includes
#include "ibc/qt/gl_point_cloud_view.h"
#include "ibc/base/log.h"
#include "ibc/gl/data.h"
#include "ibc/gl/file/ply.h"

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
    mDataNum = width * height;
    mData = new ibc::gl::glXYZf_RGBAub[mDataNum];
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
        mData[width * i + j].r = (GLubyte )(z * 255);
        mData[width * i + j].g = (GLubyte )(z * 255);
        mData[width * i + j].b = (GLubyte )(z * 255);
        mData[width * i + j].a = 255;
      }
    mGLView->setDataPtr((float *)mData, mDataNum);

    ui.setupUi(this);
    setCentralWidget(mGLView);
  }

private:
  Ui::qpcvClass ui;

protected:
  // Member variables ----------------------------------------------------------
  ibc::qt::GLPointCloudView *mGLView;
  ibc::gl::glXYZf_RGBAub *mData;
  size_t  mDataNum;

  // Member functions ----------------------------------------------------------
  bool  readPLY(const char *inFileName)
  {
    ibc::gl::file::PLYHeader  *header;
    void  *fileDataPtr;
    size_t  fileDataSize;
    char  *headerStrBufPtr;

    if (mData != NULL)
    {
      delete mData;
      mData = NULL;
      mDataNum = 0;
    }
    if (ibc::gl::file::PLYFile::readHeader(inFileName, &header, &fileDataPtr,
                                       &fileDataSize, &headerStrBufPtr) == false)
    {
      return false;
    }
    header->debugDumpHeader(&std::cout);
    if (ibc::gl::file::PLYFile::get_glXYZf_RGBAub(*header, fileDataPtr, fileDataSize,
                                                 &mData, &mDataNum) == false)
    {
      return false;
    }
    GLfloat param[4];
    ibc::gl::file::PLYFile::calcFitParam_glXYZf_RGBAub(mData, mDataNum, param);
    mGLView->setDataPtr((float *)mData, mDataNum);
    mGLView->setModelFitParam(param);
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

#endif  // #ifdef QPCV_WINDOW_H_

