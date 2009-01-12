#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include "httpconnection.h"

extern QWaitCondition gwc;
extern QMutex gmutex;


class ImageDownload : public HttpConnection {
  Q_OBJECT
public:
  int count;
  
  ImageDownload();
  ~ImageDownload();
  void blockingThing();
  void syncGet( const QString& path, bool isSync = false );


private:
  QImage *userImage;
  QEventLoop m_eventLoop;

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestStarted( int requestId );
  
signals:  
  void imageDownloaded( const QString&, const QImage& );
  void quitLoop();
};

#endif //IMAGEDOWNLOAD_H
