#include "imagedownload.h"


ImageDownload::ImageDownload() : HttpConnection() {}

void ImageDownload::run() {
  for (int i = 0; i < count; i++) {
    qDebug() << "Locking image mutex...";
    gmutex.lock();
    gwc.wait( &gmutex );
    qDebug() << "Setting global condition in imageDownload...";
    qDebug() << "IMAGEDOWNLOAD: running download for " << url.toString();
    //mutex.lock();
    //get( url.toString() );
    qDebug() << "IMAGEDOWNLOAD: getting... ";
    //wc.wait( &mutex );
    gwc.wakeAll();
    gmutex.unlock();
  }
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << "Response for" << url.path();
  qDebug() << "Code is:" << responseHeader.statusCode() << ", status is:" << responseHeader.reasonPhrase() << "\n";
  switch (responseHeader.statusCode()) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    userImage = new QImage( ":/icons/icons/noimage.png" );
    emit imageDownloaded( url.toString(), *userImage );
    delete userImage;
    userImage = 0;
    break;
  default:
    emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    http->abort();
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
    wc.wakeAll();
    mutex.unlock();
  }
}

void ImageDownload::httpRequestFinished(int requestId, bool error)
{
  //requestFinished( requestId, error );
  if (requestId != httpGetId)
    return;
  if (httpRequestAborted) {
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
    return;
  }
  if (requestId != httpGetId)
    return;
  
  buffer->close(); 
  
  if (error) {
    emit errorMessage( tr("Download failed: ") + http->errorString() );
  }
  userImage = new QImage();
  userImage->loadFromData( *bytearray, "jpg" );
  emit imageDownloaded( url.toString(), *userImage );
  wc.wakeAll();
  mutex.unlock();
  delete userImage;
  userImage = 0;

  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;    
}

