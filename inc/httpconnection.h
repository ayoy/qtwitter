#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "ui_authdialog.h"
#include "entry.h"

#include <QtNetwork>

class HttpConnection : public QHttp {

  Q_OBJECT

public:
  HttpConnection( QObject *parent = 0 );
  virtual ~HttpConnection();
  virtual void syncGet( const QString &path, bool isSync = false, QStringList cookie = QStringList() );
  void syncPost( const QString &path, const QByteArray &status, bool isSync = false, QStringList cookie = QStringList() );
  inline void setUrl( const QString &path );
  
public slots:
  virtual void httpRequestFinished( int requestId, bool error ) = 0;
  virtual void readResponseHeader( const QHttpResponseHeader &responseHeader ) = 0;
  virtual void httpRequestStarted( int requestId );

signals:
  void dataParsed( const QString& );
  void newEntry( const Entry&, int );
  void errorMessage( const QString& );

protected:
  QByteArray prepareRequest( const QString &path );
  QByteArray *bytearray;
  QBuffer *buffer;
  QUrl url;
  bool httpRequestAborted;
  int httpGetId;
  int httpHostId;
  int httpUserId;
  int closeId;
};

#endif //HTTPCONNECTION_H
