#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include "httpconnection.h"
#include "xmlparser.h"
#include <QAuthenticator>

class Core;

class XmlDownload : public HttpConnection {
  Q_OBJECT

public:
  XmlDownload( QAuthenticator _authData, Core *coreParent, bool isForGet = false, QObject *parent = 0 );
  XmlDownload( QAuthenticator _authData, int type, Core *coreParent, bool isForGet = false, QObject *parent = 0 );

public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void setAuthData( const QAuthenticator );
signals:
  void xmlParsed();
  void cookieReceived( const QStringList );

private:
  void createConnections( Core *whereToConnectTo, bool isForGet = false );
  QAuthenticator authData;
  XmlParser parser;
  Core *core;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
