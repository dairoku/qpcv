// =============================================================================
//  main.cpp
//
//  Written in 2019 by Dairoku Sekiguchi (sekiguchi at acm dot org)
//
//  To the extent possible under law, the author(s) have dedicated all copyright
//  and related and neighboring rights to this software to the public domain worldwide.
//  This software is distributed without any warranty.
//
//  You should have received a copy of the CC0 Public Domain Dedication along with
//  this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
// =============================================================================
/*!
  \file     main.cpp
  \author   Dairoku Sekiguchi
  \version  1.0.0
  \date     2019/05/01
  \brief
*/

#include "qpcv.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
#ifdef QPCV_APP_HIGH_DPI_SCALING
  // HiDPI setting
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
#endif
  QSurfaceFormat fmt;
  fmt.setDepthBufferSize(24);
  fmt.setSamples(4);
  fmt.setVersion(LIBIBC_OPENGL_MAJOR_VER, LIBIBC_OPENGL_MINOR_VER);
  fmt.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(fmt);

  QApplication app(argc, argv);
  QApplication::setApplicationName("qpcv");
  QApplication::setApplicationVersion("1.0");

  app.setStyle(QStyleFactory::create("Fusion"));

  QCommandLineParser  parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", QApplication::translate("main", "File to open."));
  parser.addOptions(
  {
    {"enableTestData", QApplication::translate("main", "Enable the test data generation.")}  // --debug option
  });

  qpcvWindow window;

  parser.process(app);
  const QStringList args = parser.positionalArguments();
  if (args.isEmpty() == false)
  {
    window.mAppOptFileNameSpecified = true;
    window.mFileName = args[0];
  }
  if (parser.isSet("enableTestData"))
  {
    window.mAppOptEnaleTestData = true;
  }

  window.show();
  return app.exec();
}
