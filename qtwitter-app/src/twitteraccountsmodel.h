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


#ifndef TWITTERACCOUNTSMODEL_H
#define TWITTERACCOUNTSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QMetaType>

struct TwitterAccount
{
  enum Type {
    SERVICE_TWITTER,
    SERVICE_IDENTICA
  };

  bool isEnabled;
  int service;
  QString login;
  QString password;
  bool directMessages;

  void setService( const QString &name ) {
    if ( name == "Twitter" )
      service = SERVICE_TWITTER;
    else if ( name == "Identi.ca" )
      service = SERVICE_IDENTICA;
  }
  QString getService() const {
    switch( service ) {
    case SERVICE_IDENTICA:
      return "Identi.ca";
    case SERVICE_TWITTER:
    default:
      return "Twitter";
    }
  }
  bool operator==( const TwitterAccount &other ) const {
    return ( isEnabled == other.isEnabled &&
             service == other.service &&
             login == other.login &&
             password == other.password &&
             directMessages == other.directMessages );
  }
};

Q_DECLARE_METATYPE(TwitterAccount)

class TwitterAccountsModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  TwitterAccountsModel( QObject *parent = 0 );

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

  QList<TwitterAccount>& getAccounts();

  // TODO: do we really need these two?
  TwitterAccount& account( int index );
  TwitterAccount* account( const QString &login );

  int indexOf( const TwitterAccount &account );

private:
  TwitterAccount emptyAccount();
  QList<TwitterAccount> accounts;
};

#endif // TWITTERACCOUNTSMODEL_H
