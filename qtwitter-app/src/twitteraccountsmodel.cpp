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


#include <QStringList>
#include "twitteraccountsmodel.h"

TwitterAccountsModel::TwitterAccountsModel( QObject *parent ) : QAbstractItemModel( parent ) {}

int TwitterAccountsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED(parent)
  if ( accounts.isEmpty() )
    return 0;
  return accounts.size();
}

int TwitterAccountsModel::columnCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent)
  return 4;
}

QModelIndex TwitterAccountsModel::index( int row, int column, const QModelIndex &parent ) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column ) : QModelIndex();
}

QModelIndex TwitterAccountsModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED(index)
  return QModelIndex();
}

QVariant TwitterAccountsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || role != Qt::DisplayRole )
    return QVariant();

  switch ( index.column() ) {
  case 0:
    return accounts.at( index.row() ).isEnabled;
  case 1:
    return accounts.at( index.row() ).login;
  case 2:
    return accounts.at( index.row() ).password;
  case 3:
    return accounts.at( index.row() ).directMessages;
  default:
    return QVariant();
  }
}

QVariant TwitterAccountsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
    return QVariant();

  switch (section) {
  case 0:
    return tr( "Enabled" );
  case 1:
    return tr( "Login" );
  case 3:
    return tr( "Direct msgs" );
  default:
    return QVariant();
  }
}

bool TwitterAccountsModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED(parent)
  if ( row > accounts.size() )
    row = accounts.size();

  TwitterAccount account;
  account.isEnabled = false;
  account.login = tr( "<empty>" );
  account.password = "";
  account.directMessages = false;

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i <= row + count - 1; i++ ) {
    accounts.insert( i, account );
    this->setData( createIndex( i, 1 ), "HEHE" );
  }
  endInsertRows();
  return true;
}

bool TwitterAccountsModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED(parent)
//  if ( !parent.isValid() )
//    return false;

//  if ( row >= accounts.size() || row + count >= accounts.size() )
//    return false;

  if ( row > accounts.size() )
    return false;
  if ( row + count > accounts.size() )
    count = accounts.size() - row;

  beginRemoveRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row + count - 1; i >= row; i-- ) {
    accounts.removeAt(i);
  }
  endRemoveRows();
  return true;
}

void TwitterAccountsModel::clear()
{
  removeRows( 0, accounts.size() );
}

QList<TwitterAccount>& TwitterAccountsModel::getAccounts()
{
  return accounts;
}

TwitterAccount& TwitterAccountsModel::account( int index )
{
  return accounts[ index ];
}
