#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "ui_authdialog.h"
#include "entry.h"
#include "xmlparser.h"

#include <QtNetwork>

extern QWaitCondition gwc;

class HttpConnection : public QThread {

  Q_OBJECT

public:
  HttpConnection();
  void get( const QString &path );
  void post( const QString &path, const QByteArray &status );
  void setUrl( const QString &path );
  QWaitCondition wc;
  QMutex mutex;
  
  
protected:
  void requestFinished( int requestId, bool error );
  
public slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader );
  
  void httpRequestStarted( int requestId );
  void updateDataReadProgress( int bytesRead, int totalBytes );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void forwardDataParsed( const QString& );
  void forwardNewEntry( const Entry& );

signals:
  void dataParsed( const QString& );
  void newEntry( const Entry& );
  void imageDownloaded( const QString&, const QImage& );
  void errorMessage( const QString& );

protected:
  virtual void run();

  bool got;
  QHttp *http;
  QUrl url;
  QByteArray *bytearray;
  QBuffer *buffer;
  QNetworkProxy proxy;
  bool httpRequestAborted;
  int httpGetId;
};

#endif //HTTPCONNECTION_H
