#include "imagedownload.h"

ImageDownload::ImageDownload() : HttpConnection() {
  userImage = 0;
  //connect( this, SIGNAL(imageDownloaded( const QString&, const QImage& )), m_eventLoop, SLOT( quit() ));
  //connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), m_eventLoop, SLOT( quit() ));
  connect( this, SIGNAL(quitLoop()), &m_eventLoop, SLOT( quit() ) );
}

ImageDownload::~ImageDownload() {
  if ( userImage ) {
    delete userImage;
    userImage = 0;
  }
}

void ImageDownload::syncGet( const QString &path, bool isSync )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = get( encodedPath, buffer );
  if ( isSync ) {
    qDebug() << "szoke";
    m_eventLoop.exec( QEventLoop::ExcludeUserInputEvents );
    qDebug() << "poczekane";
  }
  qDebug() << httpGetId;
}

void ImageDownload::httpRequestStarted( int requestId ) {
  qDebug() << httpHostId << requestId << "(in ImageDownload)";
  if ( requestId == httpHostId ) {
    qDebug() << "setHost()";
    return;
  }
  if ( requestId == httpUserId ) {
    qDebug() << "setUser()";
    return;
  }
  if ( requestId == httpGetId ) {
    qDebug() << "get()";
    return;
  }
  if ( requestId == closeId ) {
    qDebug() << "close()";
  }
  //  if ( requestId != id )
  qDebug() << "The request" << requestId << "has started\n" << url.toString();
}

void ImageDownload::blockingThing() {
  //connect( this, SIGNAL( imageDownloaded( const QString&, const QImage& ) ), m_eventLoop, SLOT( quit() ));
  //connect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), m_eventLoop, SLOT( quit() ));
  //connect( this, SIGNAL( quitLoop() ), m_eventLoop, SLOT( quit() ));

  syncGet( url.toString(), true );

  //disconnect( this, SIGNAL(imageDownloaded( const QString&, const QImage& )), m_eventLoop, SLOT( quit() ));
  //disconnect( http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), m_eventLoop, SLOT( quit() ));
  //disconnect( this, SIGNAL( quitLoop() ), m_eventLoop, SLOT( quit() ));
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
    abort();
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
  closeId = close();
  qDebug() << "HTTP REQUEST FINISHED - ID:" << requestId;
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
  
  m_eventLoop.exit();
  buffer->close();
  
  if (error) {
    emit errorMessage( tr("Download failed: ") + errorString() );
  }
  userImage = new QImage();
  userImage->loadFromData( *bytearray, "jpg" );
  qDebug() << "got it";
  emit imageDownloaded( url.toString(), *userImage );
  delete userImage;
  userImage = 0;

  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;
  m_eventLoop.exit();
//  emit quitLoop();
}
