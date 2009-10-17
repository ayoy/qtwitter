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
#include <QList>
#include <twitterapi/twitterapi_global.h>
#include "account.h"

class AccountsModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    enum Columns {
        COL_ENABLED = 0,
        COL_NETWORK,
        COL_LOGIN,
        COL_PASSWORD,
        COL_DM
    };

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
    void cleanUp();

    QList<Account> getAccounts() const;
    void setAccounts( const QList<Account> &accounts );

    // TODO: do we really need these two?
    Account& account( int index );
    Account* account( const QString &serviceUrl, const QString &login );

    int indexOf( const Account &account );

private:
    static Account emptyAccount();
    QList<Account> accounts;
};

#endif // ACCOUNTSMODEL_H
