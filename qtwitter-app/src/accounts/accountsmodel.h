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


#ifndef ACCOUNTSMODEL_H
#define ACCOUNTSMODEL_H

#include <QAbstractItemModel>
#include <QRegExp>
#include <QList>
#include <QMetaType>
#include <twitterapi/twitterapi_global.h>

struct Account
{
  bool isEnabled;
  TwitterAPI::SocialNetwork network;
  QString login;
  QString password;
  bool directMessages;

  static QPair<TwitterAPI::SocialNetwork,QString> fromString( const QString &name )
  {
    QRegExp rx( "(.+) @ (.+)" );
    if ( rx.indexIn( name ) == -1 )
      return QPair<TwitterAPI::SocialNetwork,QString>();
    return QPair<TwitterAPI::SocialNetwork,QString>( networkFromString(rx.cap(2)), rx.cap(1) );
  }

  static TwitterAPI::SocialNetwork networkFromString( const QString &name )
  {
    if ( name == "Twitter" )
      return TwitterAPI::SOCIALNETWORK_TWITTER;
    // TODO: return some error code?
    return TwitterAPI::SOCIALNETWORK_IDENTICA;
  }

  static const Account publicTimeline( TwitterAPI::SocialNetwork network )
  {
    Account account;
    account.isEnabled = true;
    account.network = network;
    account.login = TwitterAPI::PUBLIC_TIMELINE;
    account.password = "";
    account.directMessages = false;
    return account;
  }

  QString toString() const
  {
    return QString( "%1 @ %2" ).arg( login, networkToString() );
  }
  QString networkToString() const
  {
    switch( network ) {
    case TwitterAPI::SOCIALNETWORK_IDENTICA:
      return "Identi.ca";
    case TwitterAPI::SOCIALNETWORK_TWITTER:
    default:
      return "Twitter";
    }
  }
  Account operator=( const Account &other )
  {
    Account account;
    account.isEnabled = other.isEnabled;
    account.network = other.network;
    account.login = other.login;
    account.password = other.password;
    account.directMessages = other.directMessages;
    return account;
  }
  bool operator==( const Account &other ) const
  {
    return ( isEnabled == other.isEnabled &&
             network == other.network &&
             login == other.login &&
             password == other.password &&
             directMessages == other.directMessages );
  }
  bool operator<( const Account &other ) const
  {
    if ( network != other.network )
      return network < other.network;
    return login < other.login;
  }

};

Q_DECLARE_METATYPE(Account)

class AccountsModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  AccountsModel( QObject *parent = 0 );

  int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  int columnCount( const QModelIndex &parent = QModelIndex() ) const;

  QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
  QModelIndex parent( const QModelIndex &index ) const;

  QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
  Qt::ItemFlags flags( const QModelIndex &index ) const;

  bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
  bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
  void clear();

  QList<Account>& getAccounts();

  // TODO: do we really need these two?
  Account& account( int index );
  Account* account( TwitterAPI::SocialNetwork network, const QString &login );

  int indexOf( const Account &account );

private:
  Account emptyAccount();
  QList<Account> accounts;
};

#endif // ACCOUNTSMODEL_H
