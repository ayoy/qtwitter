#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "xmldownload.h"
#include "xmlparser.h"
#include "xmlparserdirectmsg.h"

const QNetworkRequest::Attribute XmlDownload::ATTR_LOGIN              = (QNetworkRequest::Attribute) QNetworkRequest::User;
const QNetworkRequest::Attribute XmlDownload::ATTR_PASSWORD           = (QNetworkRequest::Attribute) (QNetworkRequest::User + 1);
const QNetworkRequest::Attribute XmlDownload::ATTR_STATUS             = (QNetworkRequest::Attribute) (QNetworkRequest::User + 2);
const QNetworkRequest::Attribute XmlDownload::ATTR_INREPLYTO_ID       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 3);
const QNetworkRequest::Attribute XmlDownload::ATTR_DM_REQUESTED       = (QNetworkRequest::Attribute) (QNetworkRequest::User + 4);
const QNetworkRequest::Attribute XmlDownload::ATTR_DELETION_REQUESTED = (QNetworkRequest::Attribute) (QNetworkRequest::User + 5);
const QNetworkRequest::Attribute XmlDownload::ATTR_DELETE_ID          = (QNetworkRequest::Attribute) (QNetworkRequest::User + 6);

XmlDownload::XmlDownload( QObject *parent ) :
    QObject( parent ),
    connection( new QNetworkAccessManager( this ) ),
    statusParser( new XmlParser( this ) ),
    directMsgParser( new XmlParserDirectMsg( this ) )
{
  connect( connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
  connect( statusParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
  connect( directMsgParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
}

XmlDownload::~XmlDownload()
{
  connection->deleteLater();
  statusParser->deleteLater();
  directMsgParser->deleteLater();
}

void XmlDownload::createConnections()
{
//  if ( role == TwitterAPI::Destroy ) {
//    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SLOT(extractId(Entry*)) );
//  } else {
    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
//  }
  connect( directMsgParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
//  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
}

void XmlDownload::postUpdate( const QString &login, const QString &password, const QString &data, int inReplyTo )
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/update.xml" ) );
  QByteArray content = prepareRequest( data, inReplyTo );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setAttribute( XmlDownload::ATTR_STATUS, data );
  request.setAttribute( XmlDownload::ATTR_INREPLYTO_ID, inReplyTo );
  request.setRawHeader( "Authorization", setAuthorizationData( login, password ) );
  connection->post( request, content );
}

void XmlDownload::deleteUpdate( const QString &login, const QString &password, int id )
{
  QNetworkRequest request( QUrl( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ) ) );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setAttribute( XmlDownload::ATTR_DELETION_REQUESTED, true );
  request.setAttribute( XmlDownload::ATTR_DELETE_ID, id );
  request.setRawHeader( "Authorization", setAuthorizationData( login, password ) );
  connection->post( request, QByteArray() );
}

void XmlDownload::friendsTimeline( const QString &login, const QString &password, bool dm )
{
  QNetworkRequest request( QUrl( "http://twitter.com/statuses/friends_timeline.xml" ) );
  request.setAttribute( XmlDownload::ATTR_LOGIN, login );
  request.setAttribute( XmlDownload::ATTR_PASSWORD, password );
  request.setRawHeader( "Authorization", setAuthorizationData( login, password ) );
  connection->get( request );
  if ( dm ) {
    request.setUrl( QUrl( "http://twitter.com/direct_messages.xml" ) );
    request.setAttribute( XmlDownload::ATTR_DM_REQUESTED, true );
    connection->get( request );
  }
}

void XmlDownload::publicTimeline()
{
  connection->get( QNetworkRequest( QUrl( "http://twitter.com/statuses/public_timeline.xml" ) ) );
}

void XmlDownload::parseXml( const QString &login, bool useDMParser )
{
  if ( useDMParser ) {
    qDebug() << "parsing direct messages data";
    source.setData( dms.value( login ) );
    xmlReader.setContentHandler( directMsgParser );
    xmlReader.parse( source );
    dms.remove( login );
  } else {
    qDebug() << "parsing statuses data";
    source.setData( statuses.value( login ) );
    xmlReader.setContentHandler( statusParser );
    xmlReader.parse( source );
    statuses.remove( login );
  }
  qDebug() << "========= XML PARSING FINISHED =========";
}

void XmlDownload::requestFinished( QNetworkReply *reply )
{
  int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QVariant *login = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_LOGIN );
  QVariant *password = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_PASSWORD );
  QVariant *status = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_STATUS );
  QVariant *inreplyto = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_INREPLYTO_ID );
  QVariant *dm = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_DM_REQUESTED );
  QVariant *del = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_DELETION_REQUESTED );
  QVariant *delId = &reply->attribute( (QNetworkRequest::Attribute) XmlDownload::ATTR_DELETE_ID );
  switch ( replyCode ) {
  case 200:                   // Ok
    if ( reply->operation() == QNetworkAccessManager::GetOperation ) {
      if ( login->isValid() ) {
        if ( del->isValid() && del->toBool() ) {
//          forDeletion.insert( login->toString(), reply->readAll() );
//
        } else if ( dm->isValid() && dm->toBool() ) {
          dms.insert( login->toString(), reply->readAll() );
          parseXml( login->toString(), true );
        } else {
          statuses.insert( login->toString(), reply->readAll() );
          parseXml( login->toString() );
        }
      } else {
        statuses.insert( "public timeline", reply->readAll() );
        parseXml( "public timeline" );
      }
    } else if ( reply->operation() == QNetworkAccessManager::PostOperation ) {
      if ( login->isValid() ) {
        statuses.insert( login->toString(), reply->readAll() );
        parseXml( login->toString() );
      }
    }
    break;
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    emit errorMessage( "Not found" );
    break;
  case 401:
    if ( del->isValid() && del->toBool() ) {
      emit unauthorized( delId->toInt() );
    } else if ( status->isValid() ) {
      emit unauthorized( status->toString(), inreplyto->toBool() );
    } else {
      emit unauthorized();
    }
    break;
  default:;
  }
  reply->close();
}

QByteArray XmlDownload::prepareRequest( const QString &data, int inReplyTo )
{
  QByteArray request( "status=" );
  QString statusText( data );
  statusText.replace( QRegExp( "&" ), "%26" );
  statusText.replace( QRegExp( "\\+" ), "%2B" );
  request.append( data.toUtf8() );
  if ( inReplyTo != -1 ) {
    request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
  }
  request.append( "&source=qtwitter" );
  qDebug() << request;
  return request;
}

QByteArray XmlDownload::setAuthorizationData( const QString &login, const QString &password )
{
  QByteArray auth( login.toUtf8() );
  auth.append(":");
  auth.append( password.toUtf8() );
  auth = auth.toBase64();
  return auth.prepend( "Basic " );
}
