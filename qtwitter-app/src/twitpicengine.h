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

  void postContent( const QAuthenticator &authData, QString photoPath, QString status );

public slots:
  void abort();

signals:
  void finished();

private slots:
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void httpRequestFinished( int requestId, bool error );

private:
  void createConnections( Core *whereToConnectTo );
  TwitPicXmlParser *replyParser;
  Core *core;
};

#endif // TWITPICENGINE_H
