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
#include <configfile.h>
#include "accountsmodel.h"

extern ConfigFile settings;

AccountsModel::AccountsModel( QObject *parent ) : QAbstractItemModel( parent )
{
  accounts.append( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) );
  accounts.append( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) );
}

int AccountsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  if ( accounts.isEmpty() )
    return 0;
  return accounts.size();
}

int AccountsModel::columnCount(const QModelIndex &parent ) const
{
  Q_UNUSED(parent);
  return 5;
}

QModelIndex AccountsModel::index( int row, int column, const QModelIndex &parent ) const
{
  return hasIndex(row, column, parent) ? createIndex(row, column ) : QModelIndex();
}

QModelIndex AccountsModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED(index);
  return QModelIndex();
}

QVariant AccountsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();
  
  switch ( index.column() ) {
  case COL_ENABLED:
    if ( role == Qt::CheckStateRole )
      return accounts.at( index.row() ).isEnabled ? Qt::Checked : Qt::Unchecked;
    break;
  case COL_NETWORK:
    if ( role == Qt::EditRole ) {
      return accounts.at( index.row() ).network;
    }
    if ( role == Qt::DisplayRole ) {
      switch ( accounts.at( index.row() ).network ) {
      case TwitterAPI::SOCIALNETWORK_IDENTICA:
        return "Identi.ca";
      case TwitterAPI::SOCIALNETWORK_TWITTER:
      default:
        return "Twitter";
      }
    }
    break;
  case COL_LOGIN:
    if ( role == Qt::DisplayRole || role == Qt::EditRole )
      return accounts.at( index.row() ).login;
    break;
  case COL_PASSWORD:
    if ( role == Qt::DisplayRole ) {
#ifdef OAUTH
      switch ( accounts.at( index.row() ).network ) {
      case TwitterAPI::SOCIALNETWORK_IDENTICA:
#ifdef Q_WS_HILDON
      return QString( accounts.at( index.row() ).password.length(), '*' );
#else
      return QString( accounts.at( index.row() ).password.length(), QChar(0x25cf) );
#endif
      case TwitterAPI::SOCIALNETWORK_TWITTER:
      default:
        return tr( "authorized" );
      }
#else
#ifdef Q_WS_HILDON
      return QString( accounts.at( index.row() ).password.length(), '*' );
#else
      return QString( accounts.at( index.row() ).password.length(), QChar(0x25cf) );
#endif
#endif
    }
    if ( role == Qt::EditRole )
      return accounts.at( index.row() ).password;
    break;
  case COL_DM:
    if ( role == Qt::CheckStateRole )
      return accounts.at( index.row() ).directMessages ? Qt::Checked : Qt::Unchecked;
  default:;
  }
  return QVariant();
}

QVariant AccountsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
    return QVariant();

  switch (section) {
  case COL_ENABLED:
    return tr( "Enabled" );
  case COL_NETWORK:
    //: "Service", i.e. Twitter or Identi.ca
    return tr( "Service" );
  case COL_LOGIN:
    return tr( "Login" );
  case COL_PASSWORD:
    return tr( "Password" );
  case COL_DM:
    //: This should be as short as possible (e.g. PW in Polish)
    return tr( "Direct msgs" );
  default:
    return QVariant();
  }
}

bool AccountsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  switch ( index.column() ) {
  case COL_ENABLED:
    accounts[ index.row() ].isEnabled = value.toBool();
    emit dataChanged( index, index );
    return true;
  case COL_NETWORK:
    accounts[ index.row() ].network = (TwitterAPI::SocialNetwork)value.toInt();
    emit dataChanged( index, index );
    return true;
  case COL_LOGIN:
    accounts[ index.row() ].login = value.toString();
    emit dataChanged( index, index );
    return true;
  case COL_PASSWORD:
    accounts[ index.row() ].password = value.toString();
    emit dataChanged( index, index );
    return true;
  case COL_DM:
    accounts[ index.row() ].directMessages = value.toBool();
    emit dataChanged( index, index );
    return true;
  default:;
  }
  return false;
}

Qt::ItemFlags AccountsModel::flags( const QModelIndex &index ) const
{
#ifdef OAUTH
  if ( accounts.at( index.row() ).network != TwitterAPI::SOCIALNETWORK_TWITTER ) {
#endif
    if ( index.column() == COL_LOGIN || index.column() == COL_PASSWORD )
      return QAbstractItemModel::flags( index ) |= Qt::ItemIsEditable;
#ifdef OAUTH
  }
#endif
  return QAbstractItemModel::flags( index );
}

bool AccountsModel::insertRows( int row, int count, const QModelIndex &parent )
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

bool AccountsModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED(parent);

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

void AccountsModel::clear()
{
  if ( accounts.size() > 0 )
    removeRows( 0, accounts.size() );
}

void AccountsModel::cleanUp()
{
  bool nextMeansDoubled;
  for ( int i = 0; i < accounts.size(); ++i ) {
    nextMeansDoubled = false;

    for ( int j = i + 1; j < accounts.size(); ) {
      if ( accounts.at(i).network == accounts.at(j).network &&
           accounts.at(i).login == accounts.at(j).login ) {

        if ( accounts.at(j).isEnabled || accounts.at(i).isEnabled ) {
          accounts[i].isEnabled = true;
        }
        removeRow( j );
        settings.deleteAccount( j, rowCount() );

      } else {
        j++;
      }
    } //for
  } //for
}

QList<Account> AccountsModel::getAccounts()
{
  return accounts;
}

Account& AccountsModel::account( int index )
{
  return accounts[ index ];
}

Account* AccountsModel::account( TwitterAPI::SocialNetwork network, const QString &login )
{
  for ( int i = 0; i < accounts.size(); i++ ) {
    if ( login == accounts[i].login && network == accounts[i].network )
      return &accounts[i];
  }
  return 0;
}

int AccountsModel::indexOf( const Account &account )
{
  return accounts.indexOf( account );
}

Account AccountsModel::emptyAccount()
{
  Account empty;
  empty.isEnabled = true;
  empty.network = TwitterAPI::SOCIALNETWORK_TWITTER;
  //: This is for newly created account - when the login isn't given yet
  empty.login = tr( "<empty>" );
  empty.password = "";
  empty.directMessages = false;
  return empty;
}
