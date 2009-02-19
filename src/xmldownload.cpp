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

XmlDownload::XmlDownload( Role role, Core *coreParent, QObject *parent ) :
    HttpConnection( parent ),
    connectionRole( role ),
    statusParser(0),
    directMsgParser(0),
    core( coreParent ),
    authenticating( false ),
    authenticated( false )
{
  statusParser = new XmlParser( this );
  if ( connectionRole == XmlDownload::RefreshAll ) {
    directMsgParser = new XmlParserDirectMsg( this );
  }
  connect( this, SIGNAL(canBeUnlocked()), SLOT(unlock()) );
  createConnections( core );
}

XmlDownload::Role XmlDownload::role() const
{
  return connectionRole;
}

void XmlDownload::createConnections( Core *coreParent )
{
  connect( statusParser, SIGNAL(dataParsed(QString)), this, SIGNAL(dataParsed(QString)));
  if ( connectionRole == Destroy ) {
    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SLOT(extractId(Entry*)) );
    connect( this, SIGNAL(deleteEntry(int)), coreParent, SIGNAL(deleteEntry(int)) );
  } else {
    connect( statusParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(newEntry(Entry*)) );
    connect( statusParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(downloadOneImage(Entry*)) );
  }

  if ( directMsgParser ) {
    connect( directMsgParser, SIGNAL(dataParsed(QString)), this, SIGNAL(dataParsed(QString)));
    connect( directMsgParser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(newEntry(Entry*)) );
  }

  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
  connect( this, SIGNAL(cookieReceived(QStringList)), coreParent, SLOT(storeCookie(QStringList)) );
}

void XmlDownload::extractId( Entry *entry )
{
  emit deleteEntry( entry->id() );
}

void XmlDownload::syncGet( const QString &path, ContentRequested content )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }
  httpGetId = get( encodedPath, buffer );
  if ( content == Statuses ) {
    statusesData.id = httpGetId;
    statusesData.buffer = buffer;
    statusesData.bytearray = bytearray;
  } else if ( content == DirectMessages ) {
    directMessagesData.id = httpGetId;
    directMessagesData.buffer = buffer;
    directMessagesData.bytearray = bytearray;
  }
  qDebug() << "Request of type GET and id" << httpGetId << "started";
}

void XmlDownload::syncPost( const QString &path, const QByteArray &status, ContentRequested content )
{
  QByteArray encodedPath = prepareRequest( path );
  if ( encodedPath == "invalid" ) {
    httpRequestAborted = true;
    return;
  }

  httpGetId = post( encodedPath, status, buffer );
  if ( content == Statuses ) {
    statusesData.id = httpGetId;
    statusesData.buffer = buffer;
    statusesData.bytearray = bytearray;
  } else if ( content == DirectMessages ) {
    directMessagesData.id = httpGetId;
    directMessagesData.buffer = buffer;
    directMessagesData.bytearray = bytearray;
  }
  qDebug() << "Request of type POST and id" << httpGetId << "started";
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
    if ( connectionRole == Destroy ) {
      QRegExp rx( "/statuses/destroy/(\\d+)\\.xml", Qt::CaseInsensitive );
      rx.indexIn( url.path() );
      emit deleteEntry( rx.cap(1).toInt() );
    }
  default:
    //emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();
    if ( currentId() == statusesData.id ) {
      if (statusesData.buffer) {
        statusesData.buffer->close();
        delete statusesData.buffer;
        statusesData.buffer = 0;
      }
      if(statusesData.bytearray) {
        delete statusesData.bytearray;
        statusesData.bytearray = 0;
      }
    } else if ( currentId() == directMessagesData.id ) {
      if (directMessagesData.buffer) {
        directMessagesData.buffer->close();
        delete directMessagesData.buffer;
        directMessagesData.buffer = 0;
      }
      if(directMessagesData.bytearray) {
        delete directMessagesData.bytearray;
        directMessagesData.bytearray = 0;
      }
    }
  }
}

void XmlDownload::httpRequestFinished(int requestId, bool error)
{
  closeId = close();
  if (httpRequestAborted) {
    if ( requestId == statusesData.id ) {
      if (statusesData.buffer) {
        statusesData.buffer->close();
        delete statusesData.buffer;
        statusesData.buffer = 0;
      }
      if(statusesData.bytearray) {
        delete statusesData.bytearray;
        statusesData.bytearray = 0;
      }
    } else if ( requestId == directMessagesData.id ) {
      if (directMessagesData.buffer) {
        directMessagesData.buffer->close();
        delete directMessagesData.buffer;
        directMessagesData.buffer = 0;
      }
      if(directMessagesData.bytearray) {
        delete directMessagesData.bytearray;
        directMessagesData.bytearray = 0;
      }
    }
    qDebug() << "request aborted";
    authenticated = false;
    return;
  }
  if (requestId != statusesData.id && requestId != directMessagesData.id )
    return;
  
  buffer->close(); 
  
  if (error) {
    emit errorMessage( "Download failed: " + errorString() );
  } else {
    QXmlSimpleReader xmlReader;
    QXmlInputSource source;
    qDebug() << requestId << statusesData.id << directMessagesData.id;
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
    qDebug() << "========= XML PARSING FINISHED =========";
  }
  
  if ( requestId == statusesData.id ) {
    if (statusesData.buffer) {
      delete statusesData.buffer;
      statusesData.buffer = 0;
    }
    if(statusesData.bytearray) {
      delete statusesData.bytearray;
      statusesData.bytearray = 0;
    }
  } else if ( requestId == directMessagesData.id ) {
    if (directMessagesData.buffer) {
      delete directMessagesData.buffer;
      directMessagesData.buffer = 0;
    }
    if(directMessagesData.bytearray) {
      delete directMessagesData.bytearray;
      directMessagesData.bytearray = 0;
    }
  }
  authenticated = false;
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  if ( authenticating ) {
    return;
  }
  qDebug() << "auth required";
  if ( authenticated ) {
    if ( !core->authDataDialog() ) {
      httpRequestAborted = true;
      abort();
    }
    emit canBeUnlocked();
    qDebug() << "auth dialog";
  }
//  if ( !authData.user().isEmpty() && !authData.password().isEmpty() ) {
    *authenticator = core->getAuthData();
    authenticated = true;
//  }
}

void XmlDownload::unlock()
{
  authenticating = false;
}

void XmlDownload::lock()
{
  authenticating = true;
}
