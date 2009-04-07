#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include <QObject>
#include <QMap>
#include <QNetworkRequest>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include "twitterapi.h"

class QNetworkAccessManager;
class QNetworkReply;
class XmlParser;
class XmlParserDirectMsg;

class XmlDownload : public QObject
{
  Q_OBJECT

public:
  XmlDownload( QObject *parent = 0 );
  virtual ~XmlDownload();

  void postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo = -1 );
  void deleteUpdate( const QString &login, const QString &password, int id );
  void friendsTimeline( const QString &login, const QString &password, bool dm = false );
  void publicTimeline();
  void parseXml( const QString &login, bool useDMParser = false );

signals:
  void finished( TwitterAPI::ContentRequested content );
  void errorMessage( const QString &message );
  void unauthorized();
  void unauthorized( const QString &status, int inReplyToId );
  void unauthorized( int destroyId );
  void newEntry( Entry *entry );
  void deleteEntry( int id );

private slots:
  void requestFinished( QNetworkReply *reply );

private:
  void createConnections();
  QByteArray prepareRequest( const QString &data, int inReplyTo );
  QByteArray setAuthorizationData( const QString &login, const QString &password );
  QNetworkAccessManager *connection;
  QMap<QString,QByteArray> statuses;
  QMap<QString,QByteArray> dms;
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  QXmlSimpleReader xmlReader;
  QXmlInputSource source;
  QAuthenticator authData;

  static const QNetworkRequest::Attribute ATTR_LOGIN;
  static const QNetworkRequest::Attribute ATTR_PASSWORD;
  static const QNetworkRequest::Attribute ATTR_STATUS;
  static const QNetworkRequest::Attribute ATTR_INREPLYTO_ID;
  static const QNetworkRequest::Attribute ATTR_DM_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETION_REQUESTED;
  static const QNetworkRequest::Attribute ATTR_DELETE_ID;
};

#endif //XMLDOWNLOAD_H
