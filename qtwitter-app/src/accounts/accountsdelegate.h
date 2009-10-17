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


#ifndef ACCOUNTSDELEGATE_H
#define ACCOUNTSDELEGATE_H

#include <QStyledItemDelegate>

class AccountsController;

class AccountsDelegate : public QStyledItemDelegate
{
public:
    AccountsDelegate( QObject *parent = 0 );

    QWidget* createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData ( QWidget *editor, const QModelIndex &index ) const;
    void updateEditorGeometry ( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

private:
    AccountsController *controller;
};

#endif // ACCOUNTSDELEGATE_H
