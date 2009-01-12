#include "core.h"

QWaitCondition gwc;
QMutex gmutex;

Core::Core() : QThread(), xmlPost( XmlParser::One ) {  
  connect( &xmlGet, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &xmlPost, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &xmlGet, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
  connect( &xmlGet, SIGNAL( xmlParsed() ), this, SLOT( downloadImages() ) );
  connect( &xmlPost, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
  connect( &imageDownload, SIGNAL( imageDownloaded( const QString&, const QImage& ) ), this, SLOT( saveImage( const QString&, const QImage& ) ));
}

void Core::run() {
  qDebug() << "Will be downloading images now.";
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      qDebug() << "Oh Yes, it contains!";
      saveImage( entries[i].image(), imagesHash[ entries[i].image() ] );
    } else {
      qDebug() << "Noes, iz gonna download...";
      //gmutex.lock();
      imageDownload.setUrl( entries[i].image() );
      imageDownload.blockingThing();
      qDebug() << "finished waiting ;)";
      //gwc.wait( &gmutex );
      qDebug() << "proceeding to next entry";
    }
  }
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
  imageDownload.count = entries.size();
  start();
  wait();
  emit readyToDisplay( entries, imagesHash );
}

void Core::saveImage ( const QString &imageUrl, const QImage &image ) {
  imagesHash[ imageUrl ] = image;
  qDebug() << "setting imagesHash[" << imageUrl << "]";
  //imageDownload.wc.wakeAll();
  //mutex.unlock();
}

void Core::error( const QString &message ) {
  emit errorMessage( message );
}
