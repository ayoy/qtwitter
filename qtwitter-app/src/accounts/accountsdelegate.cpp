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


#include <QObject>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>
#include "accountscontroller.h"
#include "accountsmodel.h"
#include "accountsdelegate.h"

AccountsDelegate::AccountsDelegate( QObject *parent ) :
        QStyledItemDelegate( parent )
{
    controller = static_cast<AccountsController*>(parent);
    Q_ASSERT(controller);
}

QWidget* AccountsDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Q_UNUSED(option);

    QWidget *editor = 0;
    QComboBox *comboBox;
    QLineEdit *lineEdit;
    switch ( index.column() ) {
    case AccountsModel::COL_NETWORK:
        comboBox = new QComboBox( parent );
        comboBox->addItems( QStringList( Account::networkNames() ) );
        emit controller->comboActive( true );
        editor = comboBox;
        break;
    case AccountsModel::COL_LOGIN:
        lineEdit = new QLineEdit( parent );
        editor = lineEdit;
        break;
    case AccountsModel::COL_PASSWORD:
        lineEdit = new QLineEdit( parent );
        editor = lineEdit;
        lineEdit->setEchoMode( QLineEdit::Password );
    default:
        break;
    }
    return editor;
}

void AccountsDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
    QString text;
    QComboBox *comboBox;
    QLineEdit *lineEdit;
    switch ( index.column() ) {
    case AccountsModel::COL_NETWORK:
        text = index.model()->data( index, Qt::DisplayRole ).toString();
        comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex( comboBox->findText( text ) );
        break;
    case AccountsModel::COL_LOGIN:
    case AccountsModel::COL_PASSWORD:
        text = index.model()->data( index, Qt::EditRole ).toString();
        lineEdit = static_cast<QLineEdit*>(editor);
        lineEdit->setText( text );
    default:
        break;
    }
    return;
}

void AccountsDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    Q_UNUSED(index);
    editor->setGeometry( option.rect );
    return;
}

void AccountsDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
    QComboBox *comboBox;
    QLineEdit *lineEdit;
    switch ( index.column() ) {
    case AccountsModel::COL_NETWORK:
        comboBox = static_cast<QComboBox*>(editor);
        model->setData( index, comboBox->currentText(), Qt::EditRole );
        emit controller->comboActive( false );
        break;
    case AccountsModel::COL_LOGIN:
    case AccountsModel::COL_PASSWORD:
        lineEdit = static_cast<QLineEdit*>(editor);
        model->setData( index, lineEdit->text(), Qt::EditRole );
    default:
        break;
    }
    return;
}
