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

    enum  PointColorMode
    {
      POINT_COLOR_MODE_SINGLE   = 0,
      POINT_COLOR_MODE_MAP,
      POINT_COLOR_MODE_FILE
    };

    enum  ColorMapAxis
    {
      COLOR_MAP_AXIS_X   = 0,
      COLOR_MAP_AXIS_Y,
      COLOR_MAP_AXIS_Z
    };

public:
  // Constructors and Destructor -----------------------------------------------
  // ---------------------------------------------------------------------------
  // qpcvWindow
  // ---------------------------------------------------------------------------
  qpcvWindow(QWidget *parent = Q_NULLPTR)
  : QMainWindow(parent)
  {
    // Initialize app parameters
    mAppInitCalled = false;
    mAppOptFileNameSpecified = false;
    mAppOptEnaleTestData = false;

    // Initialize data related variables
    mData = NULL;
    mDataNum = 0;
    mGLView = new ibc::qt::GLPointCloudView();

    // Initialize UI
    mUI.setupUi(this);
    setCentralWidget(mGLView);
    initPointColorModeUI();
    initColorMapUI();
  }
  // ---------------------------------------------------------------------------
  // ~qpcvWindow
  // ---------------------------------------------------------------------------
  virtual ~qpcvWindow()
  {
    if (mData != NULL)
      delete mData;
  }
  // Member variables ----------------------------------------------------------
  bool  mAppOptFileNameSpecified;
  bool  mAppOptEnaleTestData;
  QString mFileName;

protected:
  // Member variables ----------------------------------------------------------
  bool  mAppInitCalled;
  ibc::qt::GLPointCloudView *mGLView;
  ibc::gl::glXYZf_RGBAub *mData;
  size_t  mDataNum;

  bool  mHasColorData;
  PointColorMode   mPointColorMode;

  ibc::image::ColorMap::ColorMapIndex mColorMapIndex;
  std::vector<ibc::image::ColorMap::ColorMapIndex>  mColorMapIndexTable;
  ColorMapAxis  mColorMapAxis;
  int mColorMapRepeatNum;
  double  mColorMapFrom;
  double  mColorMapTo;

  // Member functions ----------------------------------------------------------
  // ---------------------------------------------------------------------------
  // openFile
  // ---------------------------------------------------------------------------
  bool openFile(bool *outIsCanceled)
  {
    QString fileName = QFileDialog::getOpenFileName(
                                        this,
                                        tr("Open PLY file"),
                                        "",
                                        tr("PLY File (*.ply);;All Files (*)"));
    if (fileName == "")
    {
      *outIsCanceled = true;
      return false;
    }

    *outIsCanceled = false;
    return readPLY(fileName.toStdString().c_str());
  }
  // ---------------------------------------------------------------------------
  // readPLY
  // ---------------------------------------------------------------------------
  bool  readPLY(const char *inFileName)
  {
    ibc::gl::file::PLYHeader  *header;
    unsigned char *fileDataPtr;
    size_t  fileDataSize;
    char  *headerStrBufPtr = NULL;

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
      delete header;
      return false;
    }
    GLfloat param[4], minMax[6];
    ibc::gl::file::PLYFile::calcFitParam_glXYZf_RGBAub(mData, mDataNum, param, minMax);
    mGLView->setDataPtr((float *)mData, mDataNum);
    mGLView->setModelFitParam(param);

    QString fileName(inFileName);
    QFileInfo fileInfo(fileName);

    mUI.mFileName->setText(fileInfo.fileName());
    mUI.mFilePath->setText(fileInfo.absolutePath());
    mUI.mFileSize->setText(QString("%1 bytes").arg(fileInfo.size()));
    mUI.mFileCreated->setText(fileInfo.created().toString());
    mUI.mFileModified->setText(fileInfo.lastModified().toString());
    //
    mUI.mPLYPointsNum->setText(QString("%1").arg(mDataNum));
    std::string str = header->getColorFormatStr(ibc::gl::file::PLYHeader::ELEMENT_TYPE_VERTEX);
    if (str.size() == 0)
    {
      mHasColorData = false;
      mPointColorMode = POINT_COLOR_MODE_MAP;
      mUI.mPLYPointColor->setText("No color data");
    }
    else
    {
      mHasColorData = true;
      mPointColorMode = POINT_COLOR_MODE_FILE;
      mUI.mPLYPointColor->setText(QString(str.c_str()));
    }
    str = header->getFormatStr();
    mUI.mPLYFormat->setText(QString(str.c_str()));
    size_t  index;
    if (header->findElementIndex(ibc::gl::file::PLYHeader::ELEMENT_TYPE_FACE, &index) == false)
      mUI.mPLYFace->setText(QString("none"));
    else
      mUI.mPLYFace->setText(QString("has face data"));
    updatePointColorModeUI();
    mUI.mPLYXMin->setText(QString("%1").arg(minMax[0]));
    mUI.mPLYXMax->setText(QString("%1").arg(minMax[1]));
    mUI.mPLYYMin->setText(QString("%1").arg(minMax[2]));
    mUI.mPLYYMax->setText(QString("%1").arg(minMax[3]));
    mUI.mPLYZMin->setText(QString("%1").arg(minMax[4]));
    mUI.mPLYZMax->setText(QString("%1").arg(minMax[5]));
    mUI.mPLYHeader->setPlainText(QString(headerStrBufPtr));

    delete headerStrBufPtr;
    delete fileDataPtr;
    delete header;

    return true;
  }
  // ---------------------------------------------------------------------------
  // generateTestData
  // ---------------------------------------------------------------------------
  bool  generateTestData()
  {
    size_t width = 640;
    size_t height = 480;
    //
    mDataNum = width * height;
    mData = new ibc::gl::glXYZf_RGBAub[mDataNum];
    if (mData == NULL)
      return false;
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

    GLfloat param[4], minMax[6];
    ibc::gl::file::PLYFile::calcFitParam_glXYZf_RGBAub(mData, mDataNum, param, minMax);
    mGLView->setDataPtr((float *)mData, mDataNum);
    mGLView->setModelFitParam(param);
    return true;
  }

  // UI related functions ------------------------------------------------------
  // ---------------------------------------------------------------------------
  // updateFileInfo
  // ---------------------------------------------------------------------------
  /*virtual void  updateFileInfo(const QString &inFileName)
  {
  }*/

  // ---------------------------------------------------------------------------
  // initPointColorModeUI
  // ---------------------------------------------------------------------------
  void  initPointColorModeUI()
  {
    mHasColorData = false;
    mPointColorMode = POINT_COLOR_MODE_SINGLE;
    //
    updatePointColorModeUI();
  }
  // ---------------------------------------------------------------------------
  // updatePointColorModeUI
  // ---------------------------------------------------------------------------
  void  updatePointColorModeUI()
  {
    mUI.mPointColorMode->clear();
    mUI.mPointColorMode->addItem(QApplication::translate("main", "Single Color"));
    mUI.mPointColorMode->addItem(QApplication::translate("main", "Color Map"));
    if (mHasColorData)
      mUI.mPointColorMode->addItem(QApplication::translate("main", "From File"));
    mUI.mPointColorMode->setCurrentIndex(mPointColorMode);
  }
  // ---------------------------------------------------------------------------
  // initColorMapUI
  // ---------------------------------------------------------------------------
  void  initColorMapUI()
  {
    std::vector<std::string>  strTable;

    ibc::image::ColorMap::getColorMapNameTable(&strTable, &mColorMapIndexTable);
    for (size_t i = 0; i < strTable.size(); i++)
      mUI.mColorMapTheme->addItem(QString(strTable[i].c_str()));
    mUI.mColorMapAxis->addItem(QString("x"));
    mUI.mColorMapAxis->addItem(QString("y"));
    mUI.mColorMapAxis->addItem(QString("z"));
    //
    mColorMapIndex = ibc::image::ColorMap::CMIndex_RainbowWide;
    mColorMapAxis = COLOR_MAP_AXIS_Z;
    mColorMapRepeatNum = 1;
    mColorMapFrom = 0;
    mColorMapTo = 0;
    //
    updateColorMapUI();
  }
  // ---------------------------------------------------------------------------
  // updateColorMapUI
  // ---------------------------------------------------------------------------
  void  updateColorMapUI()
  {
    auto index = std::find( mColorMapIndexTable.cbegin(),
                            mColorMapIndexTable.cend(), mColorMapIndex);
    mUI.mColorMapTheme->setCurrentIndex((index - mColorMapIndexTable.cbegin()));
    mUI.mColorMapAxis->setCurrentIndex(mColorMapAxis);
    mUI.mColorMapRepeatNum->setValue(mColorMapRepeatNum);
    mUI.mColorMapFrom->setValue(mColorMapFrom);
    mUI.mColorMapTo->setValue(mColorMapTo);
  }

  // Window (app) behavior implementations -------------------------------------
  // ---------------------------------------------------------------------------
  // appInit
  // ---------------------------------------------------------------------------
  virtual bool  appInit()
  {
    if (mAppOptFileNameSpecified)
      return readPLY(mFileName.toStdString().c_str());
    bool  isCanceled;
    if (openFile(&isCanceled) == false)
    {
      if (mAppOptEnaleTestData && isCanceled)
        return generateTestData();
      return false;
    }
    return true;
  }

  // Qt Event functions --------------------------------------------------------
  // ---------------------------------------------------------------------------
  // showEvent
  // ---------------------------------------------------------------------------
  virtual void showEvent(QShowEvent *event)
  {
    QWidget::showEvent(event);

    if (mAppInitCalled)
      return;
    mAppInitCalled = true;
    if (appInit() == false)
    {
      // To quit the app here, we need to do the following tricky QTimer call here.
      // Since this function is called just before the window is displayed.
      // Calling QApplication::quit() or QCoreApplication::exit(EXIT_FAILURE)
      // does not work here
      QTimer::singleShot(0, this, SLOT(close()));
    }
  }

private:
  Ui::qpcvClass mUI;

private slots:
  // ---------------------------------------------------------------------------
  // on_actionOpen_triggered
  // ---------------------------------------------------------------------------
  void on_actionOpen_triggered(void)
  {
    bool  isCanceled;
    if (openFile(&isCanceled) == false)
    {
      if (isCanceled == false)
        close();
    }
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

