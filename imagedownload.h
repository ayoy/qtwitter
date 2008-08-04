#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"

extern QWaitCondition gwc;
extern QMutex gmutex;


class ImageDownload : public HttpConnection {

public:
  ImageDownload();
  ~ImageDownload();
  int count;
  
public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  
protected:
  void run();

private:
  QImage *userImage;
};

#endif //IMAGEDOWNLOAD_H
