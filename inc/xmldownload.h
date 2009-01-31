#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include "httpconnection.h"
#include <QAuthenticator>

class XmlDownload : public HttpConnection {
  Q_OBJECT

public:
  XmlDownload( QAuthenticator _authData, QObject *whereToConnectTo, bool isForGet = false );
  XmlDownload( QAuthenticator _authData, int type, QObject *whereToConnectTo, bool isForGet = false );

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );

signals:
  void xmlParsed();
  void cookieReceived( const QStringList );

private:
  void createConnections( QObject *whereToConnectTo, bool isForGet = false );
  QAuthenticator authData;
  XmlParser parser;
};

#endif //XMLDOWNLOAD_H
