#include "imagethread.h"
#include "xmlparser.h"

QWaitCondition gwc;
QMutex gmutex;

ImageThread::ImageThread() : QThread(), upload( XmlParser::One ) {  
  connect( &imageDownload, SIGNAL( imageDownloaded( const QString&, const QImage& ) ), this, SLOT( saveImage( const QString&, const QImage& ) ));
  connect( &http, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &upload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( error( const QString& ) ) );
  connect( &http, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
  connect( &http, SIGNAL( xmlParsed() ), this, SLOT( downloadImages() ) );
  connect( &upload, SIGNAL( newEntry( const Entry&, int ) ), this, SLOT( addEntry( const Entry&, int ) ));
  connect( &imageDownload, SIGNAL( imageDownloaded( const QString&, const QImage& ) ), this, SLOT( saveImage( const QString&, const QImage& ) ));  
}

void ImageThread::run() {
  qDebug() << "Will be downloading images now.";
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      qDebug() << "Oh Yes, it contains!";
      saveImage( entries[i].image(), imagesHash[ entries[i].image() ] );
    } else {
      qDebug() << "Noes, iz gonna download...";
      gmutex.lock();
      imageDownload.setUrl( entries[i].image() );
      gwc.wakeAll();
      gwc.wait( &gmutex );
      gmutex.unlock();
    }
  }
}

void ImageThread::get( const QString &path) {
  http.get( path );
}

void ImageThread::post( const QString &path, const QByteArray &status ) {
  upload.post( path, status );
}

void ImageThread::addEntry( const Entry &entry, int type )
{
  qDebug() << "About to add new Entry";
  if ( type == XmlParser::All ) {
    if ( !xmlBeingProcessed ) {
      xmlBeingProcessed = true;
      entries.clear();
    }
    entries << entry;
  } else if ( type == XmlParser::One ) {
    entries.prepend( entry );
    qDebug() << "New entry prepended";
    downloadImages();
  }
}

void ImageThread::downloadImages() {
  xmlBeingProcessed = false;
  imageDownload.count = entries.size();
  imageDownload.start();
  start();
  imageDownload.wait();
  wait();
  qDebug() << "Now ready to display";
  emit readyToDisplay( entries, imagesHash );
}

void ImageThread::saveImage ( const QString &imageUrl, const QImage &image ) {
  imagesHash[ imageUrl ] = image;
  qDebug() << "setting imagesHash[" << imageUrl << "]";
  qDebug() << "Am I ever here?";
  //imageDownload.wc.wakeAll();
  //gettingImage.wakeAll();
  //mutex.unlock();
}

void ImageThread::error( const QString &message ) {
  emit errorMessage( message );
}
