#include "core.h"

#include "ui_authdialog.h"
#include <QSettings>

Core::Core( QObject *parent ) :
    QThread( parent ),
    downloadPublicTimeline( false ),
    xmlGet( NULL ),
    xmlPost( NULL ),
    isShowingDialog( false )
{
  connect( this, SIGNAL(xmlConnectionIdle()), SLOT(destroyXmlConnection()) );
}

Core::~Core() {}

void Core::run() {
  qDebug() << "Will be downloading" << entries.size() << "images now.";
  imageDownload = new ImageDownload;
  connect( imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SIGNAL( errorMessage( const QString& ) ) );
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      imagesHash[ entries[i].image() ] = imagesHash[ entries[i].image() ];
    } else {
      imageDownload->syncGet( entries[i].image(), true );
      imagesHash[ entries[i].image() ] = imageDownload->getUserImage();
    }
  }
  delete imageDownload;
  imageDownload = NULL;
  emit xmlConnectionIdle();
  emit readyToDisplay( entries, imagesHash );
}

void Core::setDownloadPublicTimeline( bool b ) {
  downloadPublicTimeline = b;
}

bool Core::downloadsPublicTimeline() {
  return downloadPublicTimeline;
}

void Core::get() {
   if ( downloadPublicTimeline ) {
     xmlGet = new XmlDownload ( authData, this, true );
     xmlGet->syncGet( "http://twitter.com/statuses/public_timeline.xml", false, cookie );
   } else {
     if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
       if ( !authDataDialog() ) {
         emit errorMessage( tr("Authentication is required to get your friends' updates") );
         return;
       }
     }
     if ( downloadPublicTimeline ) {
       xmlGet = new XmlDownload ( authData, this, true );
       xmlGet->syncGet( "http://twitter.com/statuses/public_timeline.xml", false, cookie );
     } else {
       xmlGet = new XmlDownload ( authData, this, true );
       xmlGet->syncGet( "http://twitter.com/statuses/friends_timeline.xml", false, cookie );
    }
  }
}

void Core::post( const QByteArray &status ) {
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( !authDataDialog() ) {
      emit errorMessage( tr("Authentication is required to post updates") );
      return;
    }
  }
  QByteArray request( "status=" );
  request.append( status );
  request.append( "&source=qtwitter" );
  qDebug() << request;
  xmlPost = new XmlDownload( authData, XmlParser::One, this );
  xmlPost->syncPost( "http://twitter.com/statuses/update.xml", request, false, cookie );
}

void Core::destroyXmlConnection() {
  if ( xmlPost ) {
    qDebug() << "destroying xmlPost";
    delete xmlPost;
    xmlPost = NULL;
  }
  if ( xmlGet ) {
    qDebug() << "destroying xmlGet";
    delete xmlGet;
    xmlGet = NULL;
  }
}

void Core::addEntry( const Entry &entry, int type )
{
  if ( type == XmlParser::All ) {
    if ( !xmlBeingProcessed ) {
      xmlBeingProcessed = true;
      entries.clear();
    }
    entries << entry;
  } else if ( type == XmlParser::One ) {
    entries.prepend( entry );
    downloadImages();
  }
}

void Core::downloadImages() {
  xmlBeingProcessed = false;
  start();
}

void Core::storeCookie( const QStringList newCookie ) {
  cookie = newCookie;
}

bool Core::authDataDialog() {
  if ( isShowingDialog )
    return true;
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  //dlg.adjustSize();
  isShowingDialog = true;

  if (dlg.exec() == QDialog::Accepted) {
    if ( ui.publicBox->isChecked() ) {
      downloadPublicTimeline = true;
      emit switchToPublic();
    } else {
      authData.setUser( ui.loginEdit->text() );
      authData.setPassword( ui.passwordEdit->text() );
      emit authDataSet( authData );
      if ( xmlGet ) {
        xmlGet->setAuthData( authData );
      }
      if ( xmlPost ) {
        xmlPost->setAuthData( authData );
      }
    }
    isShowingDialog = false;
    return true;
  }
  isShowingDialog = false;
  return false;
}

void Core::setAuthData( const QString &username, const QString &password ) {
  bool refreshTweets = false;
  if ( authData.user().compare( username ) ) {
    authData.setUser( username );
    refreshTweets = true;
  }
  if ( authData.password().compare( password ) ) {
    authData.setPassword( password );
    refreshTweets = true;
  }
  if ( refreshTweets ) {
    emit updateNeeded();
  }
}

void Core::openBrowser()
{
  QString address( "http://twitter.com/home" );
#ifdef Q_WS_MAC
  QProcess *browser = new QProcess;
  browser->start( "open " + address );
#elif defined Q_WS_X11
#elif defined Q_WS_WIN
﻿  QSettings settings( "HKEY_CLASSES_ROOT\\http\\shell\\open", QSettings::NativeFormat );
  QProcess *browser = new QProcess;
  browser->start( settings.value( "command" ).toString() + address ); 
#endif
}
