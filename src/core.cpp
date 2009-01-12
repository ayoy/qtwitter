#include "core.h"

QWaitCondition gwc;
QMutex gmutex;

Core::Core() : QThread(), xmlPost( XmlParser::One ) {
  connect( &xmlGet, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &xmlPost, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &xmlGet, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
  connect( &xmlGet, SIGNAL( xmlParsed() ), this, SLOT( downloadImages() ) );
  connect( &xmlPost, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
}

Core::~Core() {}

void Core::run() {
  qDebug() << "Will be downloading" << entries.size() << "images now.";
  imageDownload = new ImageDownload;
  //imageDownload->count = entries.size();
  connect( imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( imageDownload, SIGNAL( imageDownloaded( const QString&, QImage ) ), this, SLOT( saveImage( const QString&, QImage ) ));
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      qDebug() << "Oh Yes, it contains!";
      saveImage( entries[i].image(), imagesHash[ entries[i].image() ] );
    } else {
      qDebug() << "Noes, iz gonna download...";
      imageDownload->syncGet( entries[i].image(), true );
      saveImage( entries[i].image(), imageDownload->getUserImage() );
    }
    qDebug() << "proceeding to next entry";
  }
  delete imageDownload;
  imageDownload = NULL;
}

void Core::get( const QString &path ) {
  xmlGet.syncGet( path );
}

void Core::post( const QString &path, const QByteArray &status ) {
  xmlPost.syncPost( path, status );
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
  wait();
  emit readyToDisplay( entries, imagesHash );
}

void Core::saveImage ( const QString &imageUrl, QImage image ) {
  imagesHash[ imageUrl ] = image;
  qDebug() << "setting imagesHash[" << imageUrl << "]";
}

void Core::error( const QString &message ) {
  emit errorMessage( message );
}
