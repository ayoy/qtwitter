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

XmlDownload::XmlDownload( TwitterAPI::Role role, const QString &username, const QString &password, QObject *parent ) :
    HttpConnection( parent ),
    role( role ),
    statusParser(0),
    directMsgParser(0),
    authenticated( false )
{
  authData.setUser( username );
  authData.setPassword( password );
  statusParser = new XmlParser( this );
  directMsgParser = new XmlParserDirectMsg( this );
  createConnections();
}

XmlDownload::~XmlDownload()
{
  if ( statusParser )
    statusParser->deleteLater();
  if ( directMsgParser )
    directMsgParser->deleteLater();
}

void XmlDownload::createConnections()
{
  if ( role == TwitterAPI::Destroy ) {
    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SLOT(extractId(Entry*)) );
  } else {
    connect( statusParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
  }
  connect( directMsgParser, SIGNAL(newEntry(Entry*)), this, SIGNAL(newEntry(Entry*)) );
  connect( this, SIGNAL(authenticationRequired(QString,quint16,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QString,quint16,QAuthenticator*)));
}

void XmlDownload::getContent( const QString &path, TwitterAPI::ContentRequested content )
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

void XmlDownload::postContent( const QString &path, const QByteArray &status, TwitterAPI::ContentRequested content )
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

TwitterAPI::Role XmlDownload::getRole() const
{
  return role;
}

QString XmlDownload::getPostStatus()
{
  return postStatus;
}

int XmlDownload::getPostInReplyToId()
{
  return postInReplyToId;
}

int XmlDownload::getDestroyId()
{
  return destroyId;
}

void XmlDownload::setPostStatus( const QString &newPostStatus )
{
  postStatus = newPostStatus;
}

void XmlDownload::setPostInReplyToId( int newId )
{
  postInReplyToId = newId;
}

void XmlDownload::setDestroyId( int newId )
{
  destroyId = newId;
}

void XmlDownload::extractId( Entry *entry )
{
  emit deleteEntry( entry->id );
}

void XmlDownload::slotAuthenticationRequired(const QString & /* hostName */, quint16, QAuthenticator *authenticator)
{
  qDebug() << "auth required";
  if ( authenticated ) {
    httpRequestAborted = true;
    authenticated = false;
    abort();
    switch (role) {
    case TwitterAPI::Submit:
      emit unauthorized( postStatus, postInReplyToId );
      return;
    case TwitterAPI::Destroy:
      emit unauthorized( destroyId );
      return;
    default:
    case TwitterAPI::Refresh:
      emit unauthorized();
      return;
    }
  }
  *authenticator = authData;
  authenticated = true;
}

void XmlDownload::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
  //qDebug() << responseHeader.values() ;// allValues( "Set-Cookie" );
  switch (responseHeader.statusCode()) {
  case 200:                   // Ok
  case 301:                   // Moved Permanently
  case 302:                   // Found
  case 303:                   // See Other
  case 307:                   // Temporary Redirect
    // these are not error conditions
    break;
  case 404:                   // Not Found
    if ( role == TwitterAPI::Destroy ) {
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
    emit finished( TwitterAPI::Statuses );
  } else if ( requestId == directMessagesData.id ) {
    emit finished( TwitterAPI::DirectMessages );
  }
}

XmlData* XmlDownload::processedRequest( TwitterAPI::ContentRequested content )
{
  switch ( content ) {
    case TwitterAPI::DirectMessages:
      return &directMessagesData;
      break;
    case TwitterAPI::Statuses:
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

/*! \struct XmlData
    \brief A struct containing data handles for retrieved XML documents.

    Before creating a connection for XML data downlaod, an XmlData structure instance
    is created, and its buffer and bytearray members are assigned to corresponding
    XmlDownload class's members, that can be freed then.
*/

/*! \var int XmlData::id
    Holds the id of the request for the XML document assigned to this XmlData instance.
*/

/*! \var QBuffer* XmlData::buffer
    A pointer to the buffer.
*/

/*! \var QByteArray* XmlData::bytearray
    A pointer to the bytearray.
*/

/*! \fn XmlData::XmlData()
    A constructor that initializes members with default values.
*/

/*! \fn XmlData::~XmlData()
    A destructor that frees the allocated memory.
*/

/*! \fn void XmlData::assign( int newId, QBuffer *newBuffer, QByteArray *newByteArray )
    Assigns the id and data handles from XmlDownload class instance to the XmlData object.
    \param newId The request id.
    \param newBuffer A pointer to buffer.
    \param newByteArray A pointer to bytearray.
*/

/*! \fn void XmlData::clear()
    Frees the memory allocated by the \ref buffer and \ref bytearray.
*/

/*! \class XmlDownload
    \brief A class for downloading XML documents form Twitter REST API.

    This is a class that provides interface for downloading XML documents from Twitter.
    Includes member parsers for interpreting the downloaded data.
*/

/*! \fn XmlDownload::XmlDownload( TwitterAPI::Role role, const QString &username, const QString &password, QObject *parent = 0 )
    Creates a new object with the given \a parent. Connection \a role has to be
    specified, as well as the Core class needed to connect signals to its slots.
    \param role A connection role.
    \param username An username for authentication.
    \param password A password for authentication.
    \param parent A parent for the new object.
    \sa TwitterAPI::Role
*/

/*! \fn XmlDownload::~XmlDownload()
    A destructor.
*/

/*! \fn void XmlDownload::getContent( const QString &path, TwitterAPI::ContentRequested content )
    This method invokes HttpConnection::prepareRequest() for the \a path, assigns
    allocated data to an appropriate XmlData object and issues QHttp::get().
    \param path A path the request has to be sent to.
    \param content The content requested for this request.
    \sa postContent(), TwitterAPI::ContentRequested
*/

/*! \fn void XmlDownload::postContent( const QString &path, const QByteArray &status, TwitterAPI::ContentRequested content )
    This method invokes HttpConnection::prepareRequest() for the \a path, assigns
    allocated data to an appropriate XmlData object and issues QHttp::post()
    with parameters given as \a status. Used to post new statuses or delete
    existing ones.
    \param path A path the request has to be sent to.
    \param status A status to be posted.
    \param content The content requested for this request (should work properly
                   for both types, but currently only statuses are supported).
    \sa getContent(), TwitterAPI::ContentRequested
*/

/*! \fn TwitterAPI::Role XmlDownload::getRole() const
    Gives information about the current connection role.
    \returns The connection role.
    \sa TwitterAPI::Role
*/

/*! \fn QByteArray XmlDownload::getPostStatus()
    Returns a status to be posted to Twitter.
    \sa postContent()
*/

/*! \fn int XmlDownload::getPostInReplyToId()
    Returns an id of the status to be posted to Twitter.
    \sa postContent()
*/

/*! \fn int XmlDownload::getDestroyId()
    Returns an id of the status to be deleted.
    \sa postContent()
*/

/*! \fn void XmlDownload::setPostStatus( const QByteArray &newPostStatus )
    Sets a post request status to be \a newPostStatus.
*/

/*! \fn void XmlDownload::setPostInReplyToId( int newId )
    Sets inReplyToId to be equal to \a newId.
*/

/*! \fn void XmlDownload::setDestroyId( int newId )
    Sets an id of the status to be destroyed to be equal to \a newId.
*/

/*! \fn void XmlDownload::finished( TwitterAPI::ContentRequested content )
    Emitted for a finished request, with the content type specified as a parameter.
    \param content Tells for which content the request has finished.
*/

/*! \fn void XmlDownload::unauthorized()
    Emitted when user is unauthorized to get timeline update.
    \sa unauthorized( const QByteArray &status, int inReplyToId ), unauthorized( int destroyId )
*/

/*! \fn void XmlDownload::unauthorized( const QByteArray &status, int inReplyToId )
    Emitted when user is unauthorized to post a new status.
    \param status A status that was requested to be posted.
    \param inReplyToId Id of the status to which a reply was requested to be posted.
    \sa unauthorized(), unauthorized( int destroyId )
*/

/*! \fn void XmlDownload::unauthorized( int destroyId )
    Emitted when user is unauthorized to delete a status.
    \param destroyId Id of the status to be destroyed.
    \sa unauthorized(), unauthorized( const QByteArray &status, int inReplyToId )
*/

/*! \fn void XmlDownload::newEntry( Entry *entry )
    Emitted when a new status is parsed.
    \param entry An Entry class instance that contains status data.
*/

/*! \fn void XmlDownload::deleteEntry( int id )
    Emitted when the confirmation of the entry deletion is received.
    \param id Id of the deleted entry.
*/
