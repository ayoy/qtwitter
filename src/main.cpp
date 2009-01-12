#include "mainwindow.h"

int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  
  QTranslator translator;
  translator.load( "loc/qtwitter_" + QLocale::system().name() );
  qApp->installTranslator( &translator );
  MainWindow dlg( translator );
  
  dlg.show();

  return app.exec();
}
