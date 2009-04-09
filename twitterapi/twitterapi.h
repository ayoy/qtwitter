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


#ifndef TWITTERAPI_H
#define TWITTERAPI_H

#include <QObject>
#include <QAuthenticator>
#include "twitterapi_global.h"

class XmlDownload;
class Entry;

class TWITTERAPISHARED_EXPORT TwitterAPI : public QObject
{
  Q_OBJECT

public:
  static const QString PUBLIC_TIMELINE;

  enum Role {
    Refresh,
    Submit,
    Destroy
  };

  enum ContentRequested {
    Statuses,
    DirectMessages
  };

  TwitterAPI( QObject *parent = 0 );
  virtual ~TwitterAPI();

  bool isDirectMessagesSync();

//  bool setAuthData( const QString &user, const QString &password );
  bool setDirectMessagesSync( bool b );

public slots:
  bool get();
  bool post( QString status, int inReplyTo = -1 );
  bool destroyTweet( int id );

  void abort();
  const QAuthenticator& getAuthData() const;
  void setFlag( TwitterAPI::ContentRequested flag );

signals:
  void errorMessage( const QString &message );
  void authDataSet( const QAuthenticator &authenticator );

  void unauthorized();
  void unauthorized( const QString &status, int inReplyToId );
  void unauthorized( int destroyId );

  void addEntry( const QString &login, Entry *entry );
  void deleteEntry( const QString &login, int id );
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);
  void done();
  void timelineUpdated();
  void directMessagesSyncChanged( bool isEnabled );

private slots:
  void newEntry( const QString &login, Entry* entry );

private:
  void createConnections( XmlDownload *xmlDownload );
  bool publicTimelineSync;
  bool directMessagesSync;
  bool authDialogOpen;
  XmlDownload *xmlDownload;
  bool statusesDone;
  bool messagesDone;
};


#endif // TWITTERAPI_H
