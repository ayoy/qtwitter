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


#include <QStringList>
#include "twitteraccountsmodel.h"

TwitterAccountsModel::TwitterAccountsModel( QObject *parent ) : QAbstractItemModel( parent ) {}

TwitterAccountsModel::~TwitterAccountsModel()
{
  foreach(TwitterAccount* account, accounts)
    delete account;

  accounts.clear();
}

int TwitterAccountsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  if ( accounts.isEmpty() )
    return 0;
  return accounts.size();
}

int TwitterAccountsModel::columnCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return 4;
}

QModelIndex TwitterAccountsModel::index( int row, int column, const QModelIndex &parent ) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column, accounts[row] ) : QModelIndex();
}

QModelIndex TwitterAccountsModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED(index);
  return QModelIndex();
}

Qt::ItemFlags TwitterAccountsModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant TwitterAccountsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || role != Qt::DisplayRole )
    return QVariant();

  switch ( index.column() ) {
  case 0:
    return accounts.at( index.row() )->isEnabled;
  case 1:
    return accounts.at( index.row() )->login;
  case 2:
    return accounts.at( index.row() )->password;
  case 3:
    return accounts.at( index.row() )->directMessages;
  default:
    return QVariant();
  }
}

bool TwitterAccountsModel::setData( const QModelIndex &index, const QVariant & value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  switch ( index.column() ) {
    case 0:
      accounts.at(index.row())->isEnabled = value.toBool();
    case 1:
      accounts.at(index.row())->login = value.toString();
    case 2:
      accounts.at(index.row())->password = value.toString();
    case 3:
      accounts.at(index.row())->directMessages = value.toBool();
    default:
      return false;
  }

  emit dataChanged(index, index);
  return true;
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
    //: This should be as short as possible (e.g. PW in Polish)
    return tr( "Direct msgs" );
  default:
    return QVariant();
  }
}


bool TwitterAccountsModel::insertAccount( int row, TwitterAccount* account)
{
  if ( row > accounts.size() )
    row = accounts.size();

  beginInsertRows( QModelIndex(), row, row);
  accounts.insert( row, account );
  endInsertRows();
  return true;
}

bool TwitterAccountsModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED(parent);
  if ( row > accounts.size() )
    row = accounts.size();

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i <= row + count - 1; i++ ) {
    accounts.insert( i, emptyAccount() );
  }
  endInsertRows();
  return true;
}

bool TwitterAccountsModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED(parent);
//  if ( !parent.isValid() )
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
  if ( accounts.size() > 0 )
    removeRows( 0, accounts.size() );
}

QList<TwitterAccount*>& TwitterAccountsModel::getAccounts()
{
  return accounts;
}

TwitterAccount* TwitterAccountsModel::account( int index )
{
  return accounts.at(index);
}

TwitterAccount* TwitterAccountsModel::account( const QString &login )
{
  for ( int i = 0; i < accounts.size(); i++ ) {
    if ( login == accounts[i]->login )
      return accounts[i];
  }
  return 0;
}

int TwitterAccountsModel::indexOf( TwitterAccount* account )
{
  return accounts.indexOf( account );
}

TwitterAccount* TwitterAccountsModel::emptyAccount()
{
  TwitterAccount* empty = new TwitterAccount;
  empty->isEnabled = true;
  //: This is for newly created account - when the login isn't given yet
  empty->login = tr( "<empty>" );
  empty->password = "";
  empty->directMessages = false;
  return empty;
}
