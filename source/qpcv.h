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
    
    initPointSettingUI();
    initPointColorModeUI();
    initColorMapUI();
    initDataParamUI();
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

  ibc::image::ColorMap::ColorMapIndex mColorMapIndex;
  std::vector<ibc::image::ColorMap::ColorMapIndex>  mColorMapIndexTable;

  GLfloat mParam[4], mMinMax[6];
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
    ibc::gl::file::PLYFile::calcFitParam_glXYZf_RGBAub(mData, mDataNum, mParam, mMinMax);
    mGLView->mDataModel.setDataPtr((float *)mData, mDataNum);
    mGLView->mDataModel.setModelFitParam(mParam);
    mGLView->mDataModel.setColorMapAxis(2);
    mColorMapFrom = mMinMax[4];
    mColorMapTo   = mMinMax[5];
    calcColorMapParams();

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
      mGLView->mDataModel.setColorMode(POINT_COLOR_MODE_MAP);
      mUI.mPLYPointColor->setText("No color data");
    }
    else
    {
      mHasColorData = true;
      mGLView->mDataModel.setColorMode(POINT_COLOR_MODE_FILE);
      mUI.mPLYPointColor->setText(QString(str.c_str()));
    }
    str = header->getFormatStr();
    mUI.mPLYFormat->setText(QString(str.c_str()));
    size_t  index;
    if (header->findElementIndex(ibc::gl::file::PLYHeader::ELEMENT_TYPE_FACE, &index) == false)
      mUI.mPLYFace->setText(QString("none"));
    else
      mUI.mPLYFace->setText(QString("has face data"));

    mUI.mPLYXMin->setText(QString("%1").arg(mMinMax[0]));
    mUI.mPLYXMax->setText(QString("%1").arg(mMinMax[1]));
    mUI.mPLYYMin->setText(QString("%1").arg(mMinMax[2]));
    mUI.mPLYYMax->setText(QString("%1").arg(mMinMax[3]));
    mUI.mPLYZMin->setText(QString("%1").arg(mMinMax[4]));
    mUI.mPLYZMax->setText(QString("%1").arg(mMinMax[5]));
    mUI.mPLYHeader->setPlainText(QString(headerStrBufPtr));

    updatePointColorModeUI();
    updateColorMapUI();
    updateDataParamUI();

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

    ibc::gl::file::PLYFile::calcFitParam_glXYZf_RGBAub(mData, mDataNum, mParam, mMinMax);
    mGLView->mDataModel.setDataPtr((float *)mData, mDataNum);
    mGLView->mDataModel.setModelFitParam(mParam);
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
    //
    updatePointColorModeUI();
    //
    connect(mUI.mPointColorMode,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [=](int i)
            {
              if (i == 1)
                mUI.mColorMapGroupBox->setEnabled(true);
              else
                mUI.mColorMapGroupBox->setEnabled(false);
              bool  uiEnabled = true;
              if (i == 2)
                uiEnabled = false;
              mUI.mColorSettingLabel->setEnabled(uiEnabled);
              mUI.mPointColorLabel->setEnabled(uiEnabled);
              mUI.mPointColorBox->setEnabled(uiEnabled);
              mUI.mPointColorButton->setEnabled(uiEnabled);
              //
              mGLView->mDataModel.setColorMode(i);
              mGLView->update();
            });
  }
  // ---------------------------------------------------------------------------
  // updatePointColorModeUI
  // ---------------------------------------------------------------------------
  void  updatePointColorModeUI()
  {
    // We need to backup the setting here. Since clear() fires "currentIndexChanged"
    int colorMode = mGLView->mDataModel.getColorMode();

    mUI.mPointColorMode->clear();
    mUI.mPointColorMode->addItem(QApplication::translate("main", "Single Color"));
    mUI.mPointColorMode->addItem(QApplication::translate("main", "Color Map"));
    if (mHasColorData)
      mUI.mPointColorMode->addItem(QApplication::translate("main", "From File"));
    mUI.mPointColorMode->setCurrentIndex(colorMode);
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
    mColorMapIndex = mGLView->mDataModel.getColorMapIndex();
    mColorMapFrom = 0;
    mColorMapTo = 0;
    //
    updateColorMapUI();
    //
    connect(mUI.mColorMapTheme,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [=](int d)
            {
              mGLView->mDataModel.setColorMapIndex(mColorMapIndexTable[d]);
              mGLView->update();
            });
    connect(mUI.mColorMapAxis,
            static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [=](int d)
            {
              mGLView->mDataModel.setColorMapAxis(d);
              switch (d)
              {
                case 0:
                  mColorMapFrom = mMinMax[0];
                  mColorMapTo   = mMinMax[1];
                  break;
                case 1:
                  mColorMapFrom = mMinMax[2];
                  mColorMapTo   = mMinMax[3];
                  break;
                case 2:
                  mColorMapFrom = mMinMax[4];
                  mColorMapTo   = mMinMax[5];
                  break;
              }
              calcColorMapParams();
              mUI.mColorMapFrom->setValue(mColorMapFrom);
              mUI.mColorMapTo->setValue(mColorMapTo);
              mGLView->update();
            });
    connect(mUI.mColorMapRepeatNum,
            static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            [=](int i)
            {
              mGLView->mDataModel.setColorMapRepeatNum(i);
              mGLView->update();
            });
    connect(mUI.mColorMapFrom,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mColorMapFrom = d;
              calcColorMapParams();
              mGLView->update();
            });
    connect(mUI.mColorMapTo,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mColorMapTo = d;
              calcColorMapParams();
              mGLView->update();
            });
    connect(mUI.mUnmappedPoints,
            static_cast<void(QCheckBox::*)(bool)>(&QAbstractButton::toggled),
            this,
            [=](bool d)
            {
              mGLView->mDataModel.setColorMapUnmapMode(d);
              mGLView->update();
            });
  }
  // ---------------------------------------------------------------------------
  // updateColorMapUI
  // ---------------------------------------------------------------------------
  void  updateColorMapUI()
  {
    auto index = std::find( mColorMapIndexTable.cbegin(),
                            mColorMapIndexTable.cend(), mColorMapIndex);
    mUI.mColorMapTheme->setCurrentIndex((index - mColorMapIndexTable.cbegin()));
    mUI.mColorMapAxis->setCurrentIndex(mGLView->mDataModel.getColorMapAxis());
    mUI.mColorMapRepeatNum->setValue(mGLView->mDataModel.getColorMapRepeatNum());
    mUI.mColorMapFrom->setValue(mColorMapFrom);
    mUI.mColorMapTo->setValue(mColorMapTo);
    if (mGLView->mDataModel.getColorMapUnmapMode() == 0)
      mUI.mUnmappedPoints->setChecked(false);
    else
      mUI.mUnmappedPoints->setChecked(true);
  }
  // ---------------------------------------------------------------------------
  // initPointSettingUI
  // ---------------------------------------------------------------------------
  void  initPointSettingUI()
  {
    mUI.mPointSize->setMinimum(0.01);
    mUI.mPointColorBox->setAutoFillBackground(true);  // We already did this with the Qt Designer
    updatePointSettingUI();
    //
    connect(mUI.mPointSize,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mGLView->mDataModel.setPointSize(d);
              mGLView->update();
            });
    connect(mUI.mPointColorButton,
            static_cast<void(QAbstractButton::*)()>(&QAbstractButton::released),
            this,
            [=]()
            {
              const float *prevColor = mGLView->mDataModel.getSingleColor();
              int r = prevColor[0] * 255.0;
              int g = prevColor[1] * 255.0;
              int b = prevColor[2] * 255.0;
              //
              QColorDialog  dialog(this);
              dialog.setCurrentColor(QColor(r, g, b));
              dialog.exec();
              QColor  selColor = dialog.selectedColor();
              if (selColor.isValid() == false)
                return;
              //
              float newColor[4];
              newColor[0] = selColor.red() / 255.0;
              newColor[1] = selColor.green() / 255.0;
              newColor[2] = selColor.blue() / 255.0;
              newColor[3] = 1.0;
              mGLView->mDataModel.setSingleColor(newColor);
              updatePointSettingUI();
              mGLView->update();
            });
  }
  // ---------------------------------------------------------------------------
  // updatePointSettingUI
  // ---------------------------------------------------------------------------
  void  updatePointSettingUI()
  {
    mUI.mPointSize->setValue(mGLView->mDataModel.getPointSize());
    const float *color = mGLView->mDataModel.getSingleColor();
    int r = color[0] * 255.0;
    int g = color[1] * 255.0;
    int b = color[2] * 255.0;
    char  buf[256];
    sprintf(buf, "[%03d, %03d, %03d]", r, g, b);
    mUI.mPointColorLabel->setText(buf);

    QPalette palette = mUI.mPointColorBox->palette();
    palette.setColor(mUI.mPointColorBox->backgroundRole(), QColor(r, g, b));
    //palette.setColor(mUI.mPointColorBox->foregroundRole(), QColor(r, g, b);
    mUI.mPointColorBox->setPalette(palette);
  }
  // ---------------------------------------------------------------------------
  // calcColorMapParams
  // ---------------------------------------------------------------------------
  void  calcColorMapParams()
  {
    double  gain;
    if (mColorMapFrom >= mColorMapTo)
      gain = 0;
    else
      gain = 1.0 / (mColorMapTo - mColorMapFrom);
    mGLView->mDataModel.setColorMapOffset(mColorMapFrom);
    mGLView->mDataModel.setColorMapGain(gain);
  }
  // ---------------------------------------------------------------------------
  // initDataParamUI
  // ---------------------------------------------------------------------------
  void  initDataParamUI()
  {
    updateDataParamUI();
    //
    connect(mUI.mDataScale,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mParam[3] = d;
              mGLView->mDataModel.setModelFitParam(mParam);
              mGLView->update();
            });
    connect(mUI.mDataXOffset,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mParam[0] = d;
              mGLView->mDataModel.setModelFitParam(mParam);
              mGLView->update();
            });
    connect(mUI.mDataYOffset,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mParam[1] = d;
              mGLView->mDataModel.setModelFitParam(mParam);
              mGLView->update();
            });
    connect(mUI.mDataZOffset,
            static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this,
            [=](double d)
            {
              mParam[2] = d;
              mGLView->mDataModel.setModelFitParam(mParam);
              mGLView->update();
            });
  }
  // ---------------------------------------------------------------------------
  // updateDataParamUI
  // ---------------------------------------------------------------------------
  void  updateDataParamUI()
  {
    mUI.mDataScale->setValue(mParam[3]);
    mUI.mDataXOffset->setValue(mParam[0]);
    mUI.mDataYOffset->setValue(mParam[1]);
    mUI.mDataZOffset->setValue(mParam[2]);
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

