#include "xmldownload.h"
#include "core.h"

XmlDownload::XmlDownload( QAuthenticator _authData, Core *coreParent, bool isForGet, QObject *parent ) :
    HttpConnection( parent ),
    authData( _authData ),
    core( coreParent ),
    authenticated( false )
{
  createConnections( coreParent, isForGet );
}

XmlDownload::XmlDownload( QAuthenticator _authData, int type, Core *coreParent, bool isForGet, QObject *parent ) :
    HttpConnection( parent ),
    authData( _authData ),
    parser( type ),
    core( coreParent ),
    authenticated( false )
{
  createConnections( coreParent, isForGet );
}

void XmlDownload::createConnections( Core *coreParent, bool isForGet )
{
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SIGNAL(dataParsed(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&, int )), this, SIGNAL(newEntry(const Entry&, int )));
  connect( &parser, SIGNAL(xmlParsed()), this, SIGNAL(xmlParsed()));
  connect( this, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
  connect( this, SIGNAL( errorMessage( const QString& ) ), coreParent, SLOT( error( const QString& ) ) );
  connect( this, SIGNAL( newEntry( const Entry&, int ) ), coreParent, SLOT( addEntry( const Entry&, int ) ));
  connect( this, SIGNAL( cookieReceived( const QStringList ) ), coreParent, SLOT(storeCookie(QStringList)) );
  if ( isForGet ) {
    connect( this, SIGNAL( xmlParsed() ), coreParent, SLOT( downloadImages() ) );
  }
}

void XmlDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  //qDebug() << responseHeader.values() ;// allValues( "Set-Cookie" );
  emit cookieReceived( responseHeader.allValues( "Set-Cookie" ) );
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

void XmlDownload::httpRequestFinished(int requestId, bool error)
{
  closeId = close();
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
    qDebug() << "request aborted";
    authenticated = false;
    return;
  }
  if (requestId != httpGetId)
    return;
  
  buffer->close(); 
  
  if (error) {
    emit errorMessage( "Download failed: " + errorString() );
  } else {
    QXmlInputSource source( buffer );
    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler( &parser );
    xmlReader.parse( source );
    qDebug() << "========= XML PARSING FINISHED =========";
  }
  
  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;
  authenticated = false;
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  qDebug() << "auth required";
  if ( authenticated ) {
    core->authDataDialog();
    qDebug() << "auth dialog";
  }
  *authenticator = authData;
  authenticated = true;
}

void XmlDownload::setAuthData( const QAuthenticator _authData ) {
  authData = _authData;
}
