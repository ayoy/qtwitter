#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "ui_authdialog.h"
#include "xmlparser.h"

#include <QtNetwork>

extern QWaitCondition gwc;

class HttpConnection : public QHttp {

  Q_OBJECT

public:
  HttpConnection();
  virtual ~HttpConnection();
  virtual bool syncGet( const QString &path, bool isSync = false );
  void syncPost( const QString &path, const QByteArray &status, bool isSync = false );
  void setUrl( const QString &path );
  QWaitCondition wc;
  QMutex mutex;
  
public slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader ) = 0;
  virtual void httpRequestStarted( int requestId );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );

signals:
  void dataParsed( const QString& );
  void newEntry( const Entry&, int );
  void errorMessage( const QString& );

protected:
  //void requestFinished( int requestId, bool error );
  QByteArray prepareRequest( const QString &path );

  //QHttp *http;
  bool status;
  QByteArray *bytearray;
  QBuffer *buffer;
  QUrl url;
  QNetworkProxy proxy;
  bool httpRequestAborted;
  int httpGetId;
  int httpHostId;
  int httpUserId;
  int closeId;
};

#endif //HTTPCONNECTION_H
