#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"

class ImageDownload : public HttpConnection {
  Q_OBJECT
public:
  int count;
  
  ImageDownload();
  ~ImageDownload();
  void syncGet( const QString& path, bool isSync = false );
  QImage getUserImage();

private:
  QImage *userImage;
  QEventLoop getEventLoop;

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestStarted( int requestId );
  
signals:  
  void imageDownloaded( const QString&, QImage );
};

#endif //IMAGEDOWNLOAD_H
