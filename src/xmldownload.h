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


#ifndef XMLDOWNLOAD_H
#define XMLDOWNLOAD_H

#include <QAuthenticator>
#include "twitterapi.h"
#include "httpconnection.h"

class XmlParser;
class XmlParserDirectMsg;

/*!
  \brief A struct containing data handles for retrieved XML documents.

  Before creating a connection for XML data downlaod, an XmlData structure instance
  is created, and its buffer and bytearray members are assigned to corresponding
  XmlDownload class's members, that can be freed then.
*/
struct XmlData
{
  int id; /*!< Holds the id of the request for the XML document assigned to this XmlData instance. */
  QBuffer *buffer; /*!< A pointer to the buffer */
  QByteArray *bytearray; /*!< A pointer to the bytearray */
  XmlData();
  ~XmlData();

  /*!
    Assigns the id and data handles from XmlDownload class instance to the XmlData object.
    \param newId The request id.
    \param newBuffer A pointer to buffer.
    \param newByteArray A pointer to bytearray.
  */
  void assign( int newId, QBuffer *newBuffer, QByteArray *newByteArray );

  /*!
    Frees the memory allocated by the \ref buffer and \ref bytearray.
  */
  void clear();
};

/*!
  \brief A class for downloading XML documents form Twitter REST API.

  This is a class that provides interface for downloading XML documents from Twitter.
  Includes member parsers for interpreting the downloaded data.
*/
class XmlDownload : public HttpConnection
{
  Q_OBJECT

public:

//  /*!
//    Describes the function that the current connection has.
//  */
//  enum Role {
//    Refresh, /*!< Statuses update is requested. */
//    Submit, /*!< Posting a new status is requested. */
//    Destroy /*!< Destroying a Tweet is requested. */
//  };
//
//  /*!
//    Used to specify the content that is currently requested and has to be parsed.
//  */
//  enum ContentRequested {
//    Statuses, /*!< Statuses are requested. */
//    DirectMessages /*!< Direct messages are requested. */
//  };

  /*!
    Creates a new object with the given \a parent. Connection \a role has to be
    specified, as well as the Core class needed to connect signals to its slots.
    \param role A connection role.
    \param username An username for authentication.
    \param password A password for authentication.
    \param parent A parent for the new object.
    \sa Role
  */
  XmlDownload( TwitterAPI::Role role, const QString &username, const QString &password, QObject *parent = 0 );

  /*!
    A destructor.
  */
  ~XmlDownload();

  /*!
    This method invokes HttpConnection::prepareRequest() for the \a path, assigns
    allocated data to an appropriate XmlData object and issues QHttp::get().
    \param path A path the request has to be sent to.
    \param content The content requested for this request.
    \sa postContent(), ContentRequested
  */
  void getContent( const QString &path, TwitterAPI::ContentRequested content );

  /*!
    This method invokes HttpConnection::prepareRequest() for the \a path, assigns
    allocated data to an appropriate XmlData object and issues QHttp::post()
    with parameters given as \a status.
    \param path A path the request has to be sent to.
    \param content The content requested for this request (should work properly
                   for both types, but currently only statuses are supported).
    \sa getContent(), ContentRequested
  */
  void postContent( const QString &path, const QByteArray &status, TwitterAPI::ContentRequested content );

  /*!
    Gives information about the current connection role.
    \returns The connection role.
    \sa Role
  */
  TwitterAPI::Role getRole() const;

  QByteArray getPostStatus();
  int getPostInReplyToId();
  int getDestroyId();

  void setPostStatus( const QByteArray &newPostStatus );
  void setPostInReplyToId( int newId );
  void setDestroyId( int newId );

signals:
  /*!
    Emitted for a finished request, with the content type specified as a parameter.
    \param content Tells for which content the request has finished.
  */
  void finished( TwitterAPI::ContentRequested content );

  /*!
    Emits when cookie is recieved from the server. Not used currently, as it
    does not seem to work well at all times for Twitter REST API.
    \param cookie The received cookie.
  */
  void cookieReceived( const QStringList cookie );

  void unauthorized();
  void unauthorized( const QByteArray &status, int inReplyToId );
  void unauthorized( int destroyId );

  /*!
    Emitted when the confirmation of the entry deletion is received.
    \param id Id of the deleted entry.
  */
  void deleteEntry( int id );

  /*!
    Emitted to forward a problem with the connection or downloaded content
    to the MainWindow class instance to notify User.
    \param message An error message to be displayed.
  */
  void errorMessage( const QString &message );

private slots:
  void extractId( Entry* );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestFinished( int requestId, bool error );

signals:
  void newEntry( Entry *entry );

private:
  void createConnections();
  XmlData* processedRequest( TwitterAPI::ContentRequested content );
  XmlData* processedRequest( int requestId );
  TwitterAPI::Role role;
  QByteArray postStatus;
  int postInReplyToId;
  int destroyId;
  XmlData statusesData;
  XmlData directMessagesData;
  XmlParser *statusParser;
  XmlParserDirectMsg *directMsgParser;
  QAuthenticator authData;
  bool authenticating;
  bool authenticated;
};

#endif //XMLDOWNLOAD_H
