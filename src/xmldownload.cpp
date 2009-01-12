#include "xmldownload.h"

XmlDownload::XmlDownload() : HttpConnection() {
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SLOT(forwardDataParsed(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&, int )), this, SLOT(forwardNewEntry(const Entry&, int )));
  connect( &parser, SIGNAL(xmlParsed()), this, SLOT(forwardXmlParsed()));
//  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
}

XmlDownload::XmlDownload( int type ) : HttpConnection(), parser( type ) {
  connect( &parser, SIGNAL(dataParsed(const QString&)), this, SLOT(forwardDataParsed(const QString&)));
  connect( &parser, SIGNAL(newEntry(const Entry&, int )), this, SLOT(forwardNewEntry(const Entry&, int )));
  connect( &parser, SIGNAL(xmlParsed()), this, SLOT(forwardXmlParsed()));
//  connect( http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
}


void XmlDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
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

void XmlDownload::forwardXmlParsed() {
  qDebug() << "Document is supposed to be parsed here.";
  emit xmlParsed();
}

void XmlDownload::forwardDataParsed(const QString &data)
{
  emit dataParsed( data );
}

void XmlDownload::forwardNewEntry( const Entry &entry, int type )
{
  emit newEntry( entry, type );
}
