#include "httpconnection.h"

#include <QHttpRequestHeader>

HttpConnection::HttpConnection() : QHttp( "/*url.host()*/", QHttp::ConnectionModeHttp, 0 ),
                                   status(false), bytearray( NULL ), buffer( NULL )
{
  connect( this, SIGNAL(requestStarted(int)), this, SLOT(httpRequestStarted(int)));
  connect( this, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
  connect( this, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
}

HttpConnection::~HttpConnection() {
  if( buffer ) {
    delete buffer;
    buffer = NULL;
  }
  if( bytearray ) {
    delete bytearray;
    bytearray = NULL;
  }
}

void HttpConnection::httpRequestStarted( int /*requestId*/ ) {
  //qDebug() << currentRequest().toString();
}

void HttpConnection::setUrl( const QString &path ) {
  url.setUrl( path );
}

QByteArray HttpConnection::prepareRequest( const QString &path ) {
  url.setUrl( path );
  httpHostId = setHost( url.host(), QHttp::ConnectionModeHttp);
    
  bytearray = new QByteArray();
  buffer = new QBuffer( bytearray );

  if ( !buffer->open(QIODevice::ReadWrite) )
  {                                     
    emit errorMessage( tr("Unable to open device: ") + buffer->errorString() );
    delete buffer;
    buffer = 0;
    delete bytearray;
    bytearray = 0;
    return "invalid";
  }
  
  httpRequestAborted = false;
  QByteArray encodedPath = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
  if ( encodedPath.isEmpty() )
    encodedPath = "/";
  qDebug() << "About to download: " + encodedPath + " from: " + url.host();
  return encodedPath;
}

bool HttpConnection::syncGet( const QString &path, bool /*isSync*/, QStringList cookie )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return status;
  }
/*  QHttpRequestHeader *getHeader = new QHttpRequestHeader( "GET", QString( encodedPath ) );
  getHeader->setValue( "Host", url.host() );
  getHeader->setValue( "Connection", "Keep-Alive" );
  if ( !cookie.isEmpty() ) {
    for ( QStringList::iterator i = cookie.begin(); i != cookie.end(); ++i ) {
      getHeader->addValue( "Cookie", *i );
    }
  }
  qDebug() << "header:" << getHeader->toString() << getHeader->isValid();*/

//  httpGetId = request( *getHeader, 0, buffer );
  httpGetId = get( encodedPath, buffer );
  qDebug() << httpGetId << status;
  return status;
}

void HttpConnection::syncPost( const QString &path, const QByteArray &status, bool /*isSync*/, QStringList /*cookie = QString()*/ )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }

  httpGetId = post( encodedPath, status, buffer );
  qDebug() << httpGetId;
}
