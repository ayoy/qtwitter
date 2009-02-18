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
#include "core.h"

XmlDownload::XmlDownload( QAuthenticator _authData, Role role, Core *coreParent, QObject *parent ) :
    HttpConnection( parent ),
    connectionRole( role ),
    authData( _authData ),
    core( coreParent ),
    authenticated( false )
{
  createConnections( coreParent );
}

XmlDownload::Role XmlDownload::role() const
{
  return connectionRole;
}

void XmlDownload::createConnections( Core *coreParent )
{
  connect( &parser, SIGNAL(dataParsed(QString)), this, SIGNAL(dataParsed(QString)));
  if ( connectionRole == Destroy ) {
    connect( &parser, SIGNAL(newEntry(Entry*)), this, SLOT(extractId(Entry*)) );
    connect( this, SIGNAL(deleteEntry(int)), coreParent, SIGNAL(deleteEntry(int)) );
  } else {
    connect( &parser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(newEntry(Entry*)) );
    connect( &parser, SIGNAL(newEntry(Entry*)), coreParent, SLOT(downloadOneImage(Entry*)) );
  }
  connect( &parser, SIGNAL(xmlParsed()), this, SIGNAL(xmlParsed()));
  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
  connect( this, SIGNAL(cookieReceived(QStringList)), coreParent, SLOT(storeCookie(QStringList)) );
}

void XmlDownload::extractId( Entry *entry )
{
  emit deleteEntry( entry->id() );
}

void XmlDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  qDebug() << responseHeader.values() ;// allValues( "Set-Cookie" );
  emit cookieReceived( responseHeader.allValues( "Set-Cookie" ) );
  qDebug() << url.path();
  qDebug() << responseHeader.statusCode() << ": " << responseHeader.reasonPhrase() << "\n";
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
      QRegExp rx = QRegExp( "/statuses/destroy/(\\d+)\\.xml", Qt::CaseInsensitive );
      rx.indexIn( url.path() );
      emit deleteEntry( rx.cap(1).toInt() );
    }
  default:
    //emit errorMessage( "Download failed: " + responseHeader.reasonPhrase() );
    httpRequestAborted = true;
    abort();
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
  }
}

void XmlDownload::httpRequestFinished(int requestId, bool error)
{
  closeId = close();
  if (httpRequestAborted) {
    if (buffer) {
      buffer->close();
      delete buffer;
      buffer = 0;
    }
    if(bytearray) {
      delete bytearray;
      bytearray = 0;
    }
    qDebug() << "request aborted";
    authenticated = false;
    return;
  }
  if (requestId != httpGetId)
    return;
  
  buffer->close(); 
  
  if (error) {
    emit errorMessage( "Download failed: " + errorString() );
  } else {
    QXmlInputSource source( buffer );
    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler( &parser );
    xmlReader.parse( source );
    qDebug() << "========= XML PARSING FINISHED =========";
  }
  
  delete buffer;
  buffer = 0;
  delete bytearray;
  bytearray = 0;
  authenticated = false;
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  qDebug() << "auth required";
  if ( authenticated ) {
    if ( !core->authDataDialog() ) {
      httpRequestAborted = true;
      abort();
    }
    qDebug() << "auth dialog";
  }
  if ( !authData.user().isEmpty() && !authData.password().isEmpty() ) {
    *authenticator = authData;
    authenticated = true;
  }
}

void XmlDownload::setAuthData( const QAuthenticator _authData ) {
  authData = _authData;
}
