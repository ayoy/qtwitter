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
#include <twitterapi/twitterapi.h>
#include "tweetmodel.h"
#include "tweet.h"

struct Status {
  Entry entry;
  TweetModel::TweetState state;
  QPixmap image;
  bool operator==( const Status &other )
  {
    // FIXME: pixmap comparison?
    return ( entry == other.entry
             && state == other.state);
  }
};

Q_DECLARE_METATYPE(Status)

class StatusListPrivate;

class StatusList : public QObject
{
  Q_OBJECT

  typedef TwitterAPI::SocialNetwork SocialNetwork;

  Q_PROPERTY( SocialNetwork network READ network WRITE setNetwork )
  Q_PROPERTY( QString login READ login WRITE setLogin )
  Q_PROPERTY( bool visible READ isVisible WRITE setVisible )

public:
  StatusList( const QString &login, TwitterAPI::SocialNetwork network, QObject *parent = 0 );
  ~StatusList();

  void addStatus( Entry *entry );
  bool deleteStatus( int id );
  static void setMaxCount( int maxCount );

  void setNetwork( SocialNetwork network );
  SocialNetwork network() const;

  void setLogin( const QString &login );
  const QString& login() const;

  void setVisible( bool visible );
  bool isVisible() const;

  void setData( int index, const Status &status );
  const Status& data( int index ) const;

  const QList<Status>& getData() const;

  int size() const;

public slots:
  void slotDirectMessagesChanged( bool isEnabled );

signals:
  void statusAdded( int index );
  void dataChanged( int index );

private:
  StatusListPrivate * d;
};

#endif // STATUSLIST_H
