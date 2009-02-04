#include "mainwindow.h"
#include "settings.h"
#include "core.h"
#include "loopedsignal.h"

int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  qApp->setWindowIcon( QIcon( ":/icons/icons/twitter_48.png" ) );

  MainWindow dlg;
  LoopedSignal *loopedsignal = new LoopedSignal( &dlg );
  Core *core = new Core( &dlg );
  Settings *settings = new Settings( &dlg, loopedsignal, core, &dlg );

  QObject::connect( loopedsignal, SIGNAL(ping()), &dlg, SLOT(updateTweets()) );
  QObject::connect( &dlg, SIGNAL(get()), core, SLOT(get()) );
  QObject::connect( &dlg, SIGNAL(post(QByteArray)), core, SLOT(post(QByteArray)) );
  QObject::connect( &dlg, SIGNAL(settingsDialogRequested()), settings, SLOT( show() ) );
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settings, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(errorMessage(QString)), &dlg, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(readyToDisplay(ListOfEntries,MapStringImage)), &dlg, SLOT(display(ListOfEntries,MapStringImage)) );
  QObject::connect( core, SIGNAL(updateNeeded()), &dlg, SLOT(updateTweets()) );
  //connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );
  dlg.show();

  if ( !loopedsignal->isRunning() )
    loopedsignal->start();

  return app.exec();
}
