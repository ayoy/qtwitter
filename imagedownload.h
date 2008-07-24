#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"
                        
#include <QtNetwork>
#include <QImageReader>

class ImageDownload : public HttpConnection {

public:
  ImageDownload();
  
public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );

protected:
  void run();

private:
  QImage *userImage;
};

#endif //IMAGEDOWNLOAD_H
