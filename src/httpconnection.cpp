#include "httpconnection.h"

HttpConnection::HttpConnection() : QHttp( "/*url.host()*/", QHttp::ConnectionModeHttp, 0 ),
                                   status(false), bytearray( NULL ), buffer( NULL )
{
  //http = new QHttp( url.host(), QHttp::ConnectionModeHttp, 0, this);
  connect( this, SIGNAL(requestStarted(int)), this, SLOT(httpRequestStarted(int)));
  connect( this, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
  connect( this, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
  connect( this, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
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
//  qDebug() << httpHostId << requestId << "(in HttpConnection)";
//  qDebug() << "The request" << requestId << "has started";
/*  if ( requestId == httpHostId )
    qDebug() << "setHost()";
  else if ( requestId == httpUserId )
    qDebug() << "setUser()";*/
}

void HttpConnection::setUrl( const QString &path ) {
  url.setUrl( path );
}

QByteArray HttpConnection::prepareRequest( const QString &path ) {
  url.setUrl( path );
  //url.setUrl( "http://s3.amazonaws.com/twitter_production/profile_images/53492115/avatar2_normal.jpg" );
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
  
  if (!url.userName().isEmpty())
    httpUserId = setUser(url.userName(), url.password());
  else
    httpUserId = setUser( "", "" );
    
  httpRequestAborted = false;
  QByteArray encodedPath = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
  if ( encodedPath.isEmpty() )
    encodedPath = "/";
  qDebug() << "About to download: " + encodedPath + " from: " + url.host();
  return encodedPath;
}

bool HttpConnection::syncGet( const QString &path, bool /*isSync*/ )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return status;
  }
  httpGetId = get( encodedPath, buffer );
  qDebug() << httpGetId;
  return status;
}

void HttpConnection::syncPost( const QString &path, const QByteArray &status, bool /*isSync*/ )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = post( encodedPath, status, buffer );
  qDebug() << httpGetId;
}

void HttpConnection::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  dlg.adjustSize();
  if (dlg.exec() == QDialog::Accepted) {
    authenticator->setUser( ui.loginEdit->text() );
    authenticator->setPassword( ui.passwordEdit->text() );
  }
}
