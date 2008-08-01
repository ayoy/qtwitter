#include "imagethread.h"

ImageThread::ImageThread() : QThread() {
  connect( &imageDownload, SIGNAL( imageDownloaded( const QString&, const QImage& ) ), this, SLOT( saveImage( const QString&, const QImage& ) ));
//  connect( &http, SIGNAL( errorMessage( const QString& ) ), this, SLOT( popupError( const QString& ) ) );
//  connect( &imageDownload, SIGNAL( errorMessage( const QString& ) ), this, SLOT( popupError( const QString& ) ) );
//  connect( &http, SIGNAL( dataParsed( const QString& ) ), this, SLOT( updateText( const QString& ) ) );
  connect( &http, SIGNAL( newEntry( const Entry& ) ), this, SLOT( addEntry( const Entry& ) ));
  connect( &http, SIGNAL( xmlParsed() ), this, SLOT( downloadImages() ) );
//  connect( &imageDownload, SIGNAL( dataParsed( const QString& ) ), this, SLOT( updateText( const QString& ) ) );
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
      qDebug() << "Noah, iz gonna download...";
      QImage tempImage(":/icons/icons/noimage.png");
      imagesHash[ entries[i].image() ] = tempImage;
      imageDownload.get( entries[i].image() );
    }
  }
}

void ImageThread::addEntry( const Entry &entry )
{
  entries << entry;
}

void ImageThread::downloadImages() {
  qDebug() << "Will be downloading images now.";
  for ( int i = 0; i < entries.size(); i++ ) {
    qDebug() << i << "image: " << entries[i].image();
    if ( imagesHash.contains( entries[i].image() ) ) {
      qDebug() << "Oh Yes, it contains!";
      saveImage( entries[i].image(), imagesHash[ entries[i].image() ] );
    } else {
      qDebug() << "Noah, iz gonna download...";
      QImage tempImage(":/icons/icons/noimage.png");
      imagesHash[ entries[i].image() ] = tempImage;
      imageDownload.get( entries[i].image() );
    }
  }
  emit readyToDisplay( entries, imagesHash );
}

void ImageThread::saveImage ( const QString &imageUrl, const QImage &image ) {
  imagesHash[ imageUrl ] = image;
}
