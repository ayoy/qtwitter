#include "imagedownload.h"

ImageDownload::ImageDownload() : HttpConnection() {}

void ImageDownload::run() {
  
}

void ImageDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << url.path();
  qDebug() << responseHeader.statusCode() << ": " << responseHeader.reasonPhrase() << "\n";
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
    emit imageDownloaded( *userImage );
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
    emit errorMessage( "Download failed: " + http->errorString() );
  }
  userImage = new QImage();
  userImage->loadFromData( *bytearray, "jpg" );
  emit imageDownloaded( *userImage );
  delete userImage;
  userImage = 0;

  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;    
}
