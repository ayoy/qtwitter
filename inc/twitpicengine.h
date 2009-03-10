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


#ifndef TWITPICENGINE_H
#define TWITPICENGINE_H

#include "httpconnection.h"
#include "core.h"
#include "twitpicxmlparser.h"

class TwitPicEngine : public HttpConnection
{
  Q_OBJECT

public:
  TwitPicEngine( Core *coreParent, QObject *parent = 0 );
  ~TwitPicEngine();

  /*!
    This method invokes HttpConnection::prepareRequest() for the \a path, assigns
    allocated data to an appropriate XmlData object and issues QHttp::post()
    with parameters given as \a status.
    \param path A path the request has to be sent to.
    \param content The content requested for this request (should work properly
                   for both types, but currently only statuses are supported).
    \sa getContent(), ContentRequested
  */
  void postContent( const QAuthenticator &authData, QString photoPath, QString status );

public slots:
  void abort();

signals:
  /*!
    Emitted for a finished request, with the content type specified as a parameter.
    \param content Tells for which content the request has finished.
  */
  void finished();

  /*!
    Emitted to forward a problem with the connection or downloaded content
    to the MainWindow class instance to notify User.
    \param message An error message to be displayed.
  */
  void errorMessage( const QString &message );

private slots:
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestFinished( int requestId, bool error );

private:
  void createConnections( Core *whereToConnectTo );
  TwitPicXmlParser *replyParser;
  Core *core;
};

#endif // TWITPICENGINE_H
