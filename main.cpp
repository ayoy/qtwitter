#include "mainwindow.h"

int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  MainWindow dlg;

  dlg.show();

  return app.exec();
}
