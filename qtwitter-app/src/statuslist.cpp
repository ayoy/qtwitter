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


#include <QDebug>
#include "statuslist.h"

class StatusListPrivate
{
public:
  StatusListPrivate() :
      visible( false ),
      login( QString() ),
      network( TwitterAPI::SOCIALNETWORK_TWITTER )
  {}

  int addStatus( Entry *entry );

  QList<Status> data;
  bool visible;
  QString login;
  TwitterAPI::SocialNetwork network;
  static int maxCount;
};


int StatusListPrivate::maxCount = 0;

StatusList::StatusList( const QString &login , TwitterAPI::SocialNetwork network, QObject *parent ) :
    QObject( parent )
{
  d = new StatusListPrivate;
  d->network = network;
  d->login = login;
}

StatusList::~StatusList()
{
  delete d;
  d = 0;
}

void StatusList::setNetwork( SocialNetwork network )
{
  d->network = network;
}

StatusList::SocialNetwork StatusList::network() const
{
  return d->network;
}

void StatusList::setLogin( const QString &login )
{
  d->login = login;
}

const QString& StatusList::login() const
{
  return d->login;
}

void StatusList::setData( int index, const Status &status )
{
  d->data[ index ] = status;
  emit dataChanged( index );
}

bool StatusList::isVisible() const
{
  return d->visible;
}

void StatusList::setVisible( bool visible )
{
  d->visible = visible;
}

const Status& StatusList::data( int index ) const
{
  return d->data.at( index );
}

const QList<Status>& StatusList::getData() const
{
  return d->data;
}

int StatusList::size() const
{
  return d->data.size();
}

int StatusListPrivate::addStatus( Entry *entry )
{
  for ( QList<Status>::const_iterator i = data.begin(); i != data.end(); ++i) {
    if ( entry->id == (*i).entry.id ) {
      qDebug() << "found existing entry of the same id";
      return -1;
    }
  }

  Status status;
  status.state = TweetModel::STATE_UNREAD;
  status.entry = *entry;
  if ( status.entry.type == Entry::DirectMessage )
    status.image = QPixmap( ":/icons/mail_48.png" );

  if ( data.isEmpty() ) {
    data.append( status );
    return data.size() - 1;
  }
  for ( QList<Status>::iterator i = data.begin(); i != data.end(); ++i ) {
    if ( status.entry.id > (*i).entry.id ) {
      // TODO: not sure about 'before' - see doc on insert()
      data.insert( i, status );
      if ( data.size() >= maxCount && data.takeLast() == status )
        return -1;
      return data.indexOf( status );
    }
  }
  if ( data.size() < maxCount ) {
    data.append( status );
    return data.size() - 1;
  }
  return -1;
}

void StatusList::addStatus( Entry *entry )
{
  int index = d->addStatus( entry );
  if ( index >= 0 )
    emit statusAdded( index );
}

bool StatusList::deleteStatus( int id )
{
  for ( QList<Status>::const_iterator i = d->data.begin(); i != d->data.end(); ++i) {
    if ( id == (*i).entry.id ) {
      d->data.removeOne( *i );
      return true;
    }
  }
  return false;
}

void StatusList::slotDirectMessagesChanged( bool isEnabled )
{
  if ( isEnabled )
    return;

  for ( int i = 0; i < d->data.size(); i++ ) {
    if ( d->data.at(i).entry.type == Entry::DirectMessage ) {
      d->data.removeAt(i);
      // TODO: not sure about it
      i--;
    }
  }
}

void StatusList::setMaxCount( int maxCount )
{
  StatusListPrivate::maxCount = maxCount;
}
