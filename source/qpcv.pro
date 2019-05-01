QT += core gui widgets

TARGET = qpcv
TEMPLATE = app
DESTDIR = ./output

INCLUDEPATH += \
  ../libibc/include \
  .

HEADERS += \
  ../libibc/include/ibc/qt/gl_view.h \
  ../libibc/include/ibc/qt/gl_obj_view.h \
  ../libibc/include/ibc/qt/gl_surface_plot.h \
  qpcv.h

SOURCES += \
  main.cpp

FORMS += \
  qpcv.ui \
  open_dialog.ui
