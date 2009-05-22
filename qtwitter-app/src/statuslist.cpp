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

int StatusList::maxCount = 0;

StatusList::StatusList( const QString &login , TwitterAPI::SocialNetwork network ) :
    login( login ),
    network( network )
{}

void StatusList::setNetwork( TwitterAPI::SocialNetwork network )
{
  this->network = network;
}

TwitterAPI::SocialNetwork StatusList::getNetwork() const
{
  return network;
}

void StatusList::setLogin( const QString &login )
{
  this->login = login;
}

const QString& StatusList::getLogin() const
{
  return login;
}

void StatusList::setData( int index, const Status &status )
{
  _data[ index ] = status;
}

const Status& StatusList::data( int index ) const
{
  return _data[ index ];
}

const QList<Status>& StatusList::getData() const
{
  return _data;
}

bool StatusList::addStatus( Entry *entry )
{
  for ( QList<Status>::const_iterator i = _data.begin(); i != _data.end(); ++i) {
    if ( entry->id == (*i).entry.id ) {
      qDebug() << "found existing entry of the same id";
      return true;
    }
  }

  Status status;
  status.state = TweetModel::STATE_UNREAD;
  status.entry = *entry;
  if ( status.entry.type == Entry::DirectMessage )
    status.image = QPixmap( ":/icons/mail_48.png" );

  if ( _data.isEmpty() ) {
    _data.append( status );
    return true;
  }
  for ( QList<Status>::iterator i = _data.begin(); i != _data.end(); ++i ) {
    if ( status.entry.id > (*i).entry.id ) {
      // TODO: not sure about 'before' - see doc on insert()
      _data.insert( i, status );
      if ( _data.size() >= maxCount && _data.takeLast() == status )
        return false;
      return true;
    }
  }
  if ( _data.size() < maxCount ) {
    _data.append( status );
    return true;
  }
  return false;
}

bool StatusList::deleteStatus( int id )
{
  for ( QList<Status>::const_iterator i = _data.begin(); i != _data.end(); ++i) {
    if ( id == (*i).entry.id ) {
      _data.removeOne( *i );
      return true;
    }
  }
  return false;
}

void StatusList::slotDirectMessagesChanged( bool isEnabled )
{
  if ( isEnabled )
    return;
//  for ( int i = 0; i < _data.rowCount(); i++ ) {
//    if ( statuses[i].entry.type == Entry::DirectMessage ) {
//      if ( isVisible ) {
//        delete view->indexWidget( item(i)->index() );
//      } else {
//        delete statuses[i].tweet;
//      }
//      removeRow(i);
//      Q_ASSERT(statuses[i].tweet == 0 );
//      statuses.removeAt(i);
//      i--;
//    }
//  }
}

void StatusList::setMaxCount( int maxCount )
{
  maxCount = maxCount;
}
