/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QObject>
#include <QList>
#include <QPixmap>
#include "tweetmodel.h"
#include "tweet.h"

struct Status {
  Entry entry;
  TweetModel::TweetState state;
  QPixmap image;
  bool operator==( const Status &other )
  {
    return ( entry == other.entry
             && state == other.state);
  }
};

Q_DECLARE_METATYPE(Status)

class StatusList : public QObject
{
  Q_PROPERTY( SocialNetwork network READ getNetwork WRITE setNetwork )
  Q_PROPERTY( QString login READ getLogin WRITE setLogin )

public:
  void setNetwork( TwitterAPI::SocialNetwork network );
  TwitterAPI::SocialNetwork getNetwork() const;

  void setLogin( const QString &login );
  const QString& getLogin() const;

  void setData( int index, const Status &status );
  const Status& data( int index ) const;

  const QList<Status>& getData() const;

public:
  StatusList( const QString &login, TwitterAPI::SocialNetwork network );
  bool addStatus( Entry *entry );
  bool deleteStatus( int id );
  static void setMaxCount( int maxCount );

public slots:
  void slotDirectMessagesChanged( bool isEnabled );

private:
  QList<Status> _data;
  QString login;
  TwitterAPI::SocialNetwork network;
  static int maxCount;
};

#endif // STATUSLIST_H
