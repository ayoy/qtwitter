/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "xmldownload.h"
#include "xmlparserdirectmsg.h"
#include "core.h"

XmlData::XmlData() :
    id(-1),
    buffer(0),
    bytearray(0)
{}

XmlData::~XmlData()
{
  clear();
}

void XmlData::assign( int newId, QBuffer *newBuffer, QByteArray *newByteArray )
{
  id = newId;
  buffer = newBuffer;
  bytearray = newByteArray;
}

void XmlData::clear()
{
  if (buffer) {
    if ( buffer->isOpen() ) {
      buffer->close();
    }
    delete buffer;
    buffer = 0;
  }
  if(bytearray) {
    delete bytearray;
    bytearray = 0;
  }
}

XmlDownload::XmlDownload( Role role, Core *coreParent, QObject *parent ) :
    HttpConnection( parent ),
    role( role ),
    statusParser(0),
    directMsgParser(0),
    core( coreParent ),
    authenticated( false )
{
  statusParser = new XmlParser( this );
  if ( role == XmlDownload::RefreshAll ) {
    directMsgParser = new XmlParserDirectMsg( this );
  }
  createConnections( core );
}

XmlDownload::~XmlDownload()
{
  if ( statusParser )
    statusParser->deleteLater();
  if ( directMsgParser )
    directMsgParser->deleteLater();
}

void XmlDownload::getContent( const QString &path, XmlDownload::ContentRequested content )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = get( encodedPath, buffer );
  processedRequest( content )->assign( httpGetId, buffer, bytearray );
  bytearray = 0;
  buffer = 0;
  qDebug() << "Request of type GET and id" << httpGetId << "started" << state();
}

void XmlDownload::postContent( const QString &path, const QByteArray &status, XmlDownload::ContentRequested content )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = post( encodedPath, status, buffer );
  processedRequest( content )->assign( httpGetId, buffer, bytearray );
  bytearray = 0;
  buffer = 0;
  qDebug() << "Request of type POST and id" << httpGetId << "started";
}

XmlDownload::Role XmlDownload::getRole() const
{
  return role;
}

void XmlDownload::extractId( Entry *entry )
{
  emit deleteEntry( entry->id() );
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  qDebug() << "auth required";
  if ( authenticated ) {
    qDebug() << "auth dialog";
    switch ( core->authDataDialog() ) {
      case Core::Rejected:
        emit errorMessage( tr("Authentication is required to post updates.") );
      case Core::SwitchToPublic:
        httpRequestAborted = true;
        authenticated = false;
        abort();
        core->get();
        return;
      default:
        break;
    }
  }
  *authenticator = core->getAuthData();
  authenticated = true;
}

void XmlDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  //qDebug() << responseHeader.values() ;// allValues( "Set-Cookie" );
  //emit cookieReceived( responseHeader.allValues( "Set-Cookie" ) );
  switch (responseHeader.statusCode()) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    if ( role == Destroy ) {
      QRegExp rx( "/statuses/destroy/(\\d+)\\.xml", Qt::CaseInsensitive );
      rx.indexIn( url.path() );
      emit deleteEntry( rx.cap(1).toInt() );
    }
  default:
    qDebug() << "Download failed: " << responseHeader.reasonPhrase();
    //emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();
    processedRequest( currentId() )->clear();
  }
}

void XmlDownload::httpRequestFinished(int requestId, bool error)
{
  closeId = close();
  if (httpRequestAborted) {
    processedRequest( requestId )->clear();
    qDebug() << "request aborted";
    authenticated = false;
    return;
  }
  if (requestId != statusesData.id && requestId != directMessagesData.id )
    return;

  processedRequest( requestId )->buffer->close();

  if (error) {
    emit errorMessage( "Download failed: " + errorString() );
  } else {
    QXmlSimpleReader xmlReader;
    QXmlInputSource source;
    if ( requestId == statusesData.id ) {
      qDebug() << "parsing statuses data";
      source.setData( *statusesData.bytearray );
      xmlReader.setContentHandler( statusParser );
    } else if ( requestId == directMessagesData.id ) {
      qDebug() << "parsing direct messages data";
      source.setData( *directMessagesData.bytearray );
      xmlReader.setContentHandler( directMsgParser );
    }
    xmlReader.parse( source );
    qDebug() << "========= XML PARSING FINISHED =========" << state();
  }
  processedRequest( requestId )->clear();
  authenticated = false;
  if ( requestId == statusesData.id ) {
    emit finished( Statuses );
  } else if ( requestId == directMessagesData.id ) {
    emit finished( DirectMessages );
  }
}

void XmlDownload::createConnections( Core *coreParent )
{
  connect( this, SIGNAL(finished(XmlDownload::ContentRequested)), coreParent, SLOT(setFlag(XmlDownload::ContentRequested)) );
  connect( this, SIGNAL(errorMessage(QString)), coreParent, SIGNAL(errorMessage(QString)) );
  if ( role == Destroy ) {
    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SLOT(extractId(Entry*)) );
    connect( this, SIGNAL(deleteEntry(int)), coreParent, SIGNAL(deleteEntry(int)) );
  } else {
    connect( statusParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(newEntry(Entry*)) );
    connect( statusParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(downloadImage(Entry*)) );
  }

  if ( directMsgParser ) {
    connect( directMsgParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(newEntry(Entry*)) );
  }
  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
  connect( this, SIGNAL(cookieReceived(QStringList)), coreParent, SLOT(setCookie(QStringList)) );
}

XmlData* XmlDownload::processedRequest( XmlDownload::ContentRequested content )
{
  switch ( content ) {
    case DirectMessages:
      return &directMessagesData;
      break;
    case Statuses:
    default:
      return &statusesData;
  }
}

XmlData* XmlDownload::processedRequest( int requestId )
{
  if ( requestId == directMessagesData.id ) {
    return &directMessagesData;
  }
  return &statusesData;
}
