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


#ifndef TWITTERACCOUNTSMODEL_H
#define TWITTERACCOUNTSMODEL_H

#include <QAbstractItemModel>
#include <QList>

struct TwitterAccount
{
  bool isEnabled;
  QString login;
  QString password;
  bool directMessages;
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

  bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
  bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
  void clear();

  QList<TwitterAccount>& getAccounts();
  TwitterAccount& account( int index );

private:
  QList<TwitterAccount> accounts;
};

#endif // TWITTERACCOUNTSMODEL_H
