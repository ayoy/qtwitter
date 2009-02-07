#include "mainwindow.h"
#include "settings.h"
#include "core.h"
#include "loopedsignal.h"

//Q_IMPORT_PLUGIN(qjpeg)
//Q_IMPORT_PLUGIN(qgif)


int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  qApp->setWindowIcon( QIcon( ":/icons/icons/twitter_48.png" ) );

  MainWindow qtwitter;
  LoopedSignal *loopedsignal = new LoopedSignal( &qtwitter );
  Core *core = new Core( &qtwitter );
  Settings *settings = new Settings( &qtwitter, loopedsignal, core, &qtwitter );

  QObject::connect( loopedsignal, SIGNAL(ping()), &qtwitter, SLOT(updateTweets()) );
  QObject::connect( &qtwitter, SIGNAL(get()), core, SLOT(get()) );
  QObject::connect( &qtwitter, SIGNAL(post(QByteArray)), core, SLOT(post(QByteArray)) );
  QObject::connect( &qtwitter, SIGNAL(settingsDialogRequested()), settings, SLOT( show() ) );
  QObject::connect( core, SIGNAL(authDataSet(QAuthenticator)), settings, SLOT(setAuthDataInDialog(QAuthenticator)) ) ;
  QObject::connect( core, SIGNAL(errorMessage(QString)), &qtwitter, SLOT(popupError(QString)) );
  QObject::connect( core, SIGNAL(readyToDisplay(ListOfEntries,MapStringImage)), &qtwitter, SLOT(display(ListOfEntries,MapStringImage)) );
  QObject::connect( core, SIGNAL(updateNeeded()), &qtwitter, SLOT(updateTweets()) );
  //connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );
  qtwitter.show();

  if ( !loopedsignal->isRunning() )
    loopedsignal->start();

  return app.exec();
}
