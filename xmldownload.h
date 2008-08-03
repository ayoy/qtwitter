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
  void forwardXmlParsed();
  void forwardDataParsed( const QString& );
  void forwardNewEntry( const Entry&, int type );

signals:
  void xmlParsed();


protected:
  void run();

private:
  XmlParser parser;
};

#endif //XMLDOWNLOAD_H
