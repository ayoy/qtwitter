#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include "httpconnection.h"

class XmlDownload : public HttpConnection {
  Q_OBJECT

public:
  XmlDownload();
  XmlDownload( int type );

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

signals:
  void xmlParsed();
  void cookieReceived( const QStringList );

private:
  XmlParser parser;
};

#endif //XMLDOWNLOAD_H
