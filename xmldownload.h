#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include "httpconnection.h"
                        
#include <QtNetwork>

class XmlDownload : public HttpConnection {

public:
  XmlDownload();
  
public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

protected:
  void run();

private:
  XmlParser parser;
};

#endif //XMLDOWNLOAD_H
