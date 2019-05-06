QT += core gui widgets

TARGET = qpcv
TEMPLATE = app
DESTDIR = ./output

CONFIG += c++17
# Before Qt 5.11 we need the following too
QMAKE_CXXFLAGS += -std=c++17
# for Visual Studio the following might work (not tested)
# QMAKE_CXXFLAGS += /std::c++17

INCLUDEPATH += \
  ../libibc/include \
  .

HEADERS += \
  ../libibc/include/ibc/qt/gl_view.h \
  ../libibc/include/ibc/qt/gl_obj_view.h \
  ../libibc/include/ibc/qt/gl_surface_plot.h \
  ../libibc/include/ibc/qt/gl_point_cloud_view.h \
  qpcv.h

SOURCES += \
  main.cpp

FORMS += \
  qpcv.ui \
  open_dialog.ui
