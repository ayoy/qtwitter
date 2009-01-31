#include "xmldownload.h"

XmlDownload::XmlDownload( QAuthenticator _authData, QObject *whereToConnectTo, bool isForGet ) : HttpConnection(), authData( _authData ) {
  createConnections( whereToConnectTo, isForGet );
}

XmlDownload::XmlDownload( QAuthenticator _authData, int type, QObject *whereToConnectTo, bool isForGet ) : HttpConnection(), authData( _authData ), parser( type ) {
  createConnections( whereToConnectTo, isForGet );
}

void XmlDownload::createConnections( QObject *whereToConnectTo, bool isForGet ) {
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SIGNAL(dataParsed(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&, int )), this, SIGNAL(newEntry(const Entry&, int )));
  connect( &parser, SIGNAL(xmlParsed()), this, SIGNAL(xmlParsed()));
  connect( this, SIGNAL(authenticationRequired(const QString &, quint16, QAuthenticator *)), this, SLOT(slotAuthenticationRequired(const QString &, quint16, QAuthenticator *)));
  connect( this, SIGNAL( errorMessage( const QString& ) ), whereToConnectTo, SLOT( error( const QString& ) ) );
  connect( this, SIGNAL( newEntry( const Entry&, int ) ), whereToConnectTo, SLOT( addEntry( const Entry&, int ) ));
  connect( this, SIGNAL( cookieReceived( const QStringList ) ), whereToConnectTo, SLOT(storeCookie(QStringList)) );
  if ( isForGet ) {
    connect( this, SIGNAL( xmlParsed() ), whereToConnectTo, SLOT( downloadImages() ) );
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
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  *authenticator = authData;
}

