#include "core.h"

#include <QHttpRequestHeader>
#include "ui_authdialog.h"

Core::Core() : QThread(), xmlGet( NULL), xmlPost( NULL ) {}

Core::~Core() {}

void Core::run() {
  qDebug() << "Will be downloading" << entries.size() << "images now.";
  imageDownload = new ImageDownload;
  connect( imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( imageDownload, SIGNAL( imageDownloaded( const QString&, QImage ) ), this, SLOT( saveImage( const QString&, QImage ) ));
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      saveImage( entries[i].image(), imagesHash[ entries[i].image() ] );
    } else {
      imageDownload->syncGet( entries[i].image(), true );
      saveImage( entries[i].image(), imageDownload->getUserImage() );
    }
  }
  delete imageDownload;
  imageDownload = NULL;
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
  emit readyToDisplay( entries, imagesHash );
}

void Core::get( const QString &path ) {
  if ( authData.isNull() ) {
    authDataDialog();
  }
  xmlGet = new XmlDownload ( authData, this, true );
  xmlGet->syncGet( path, false, cookie );
}

void Core::post( const QString &path, const QByteArray &status ) {
  if ( authData.isNull() ) {
    authDataDialog();
  }
  xmlPost = new XmlDownload( authData, XmlParser::One, this );
  xmlPost->syncPost( path, status, false, cookie );
  //  xmlPost.syncPost( path, status );
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
  /*if ( xmlGet ) {
    delete xmlGet;
    xmlGet = NULL;
  }*/
  xmlBeingProcessed = false;
  start();
  //wait();
  //emit readyToDisplay( entries, imagesHash );
}

void Core::saveImage ( const QString &imageUrl, QImage image ) {
  imagesHash[ imageUrl ] = image;
  //qDebug() << "setting imagesHash[" << imageUrl << "]";
}

void Core::storeCookie( const QStringList newCookie ) {
  cookie = newCookie;
}

void Core::authDataDialog() {
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  dlg.adjustSize();
  if (dlg.exec() == QDialog::Accepted) {
    authData.setUser( ui.loginEdit->text() );
    authData.setPassword( ui.passwordEdit->text() );
  }
  emit authDataSet( authData );
}

void Core::error( const QString &message ) {
  emit errorMessage( message );
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
