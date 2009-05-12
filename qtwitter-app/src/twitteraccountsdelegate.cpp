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
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include "twitteraccountsdelegate.h"

AccountTypeDelegate::AccountTypeDelegate( QObject *parent ) : QItemDelegate( parent ) {}

QWidget* AccountTypeDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED(option);
  Q_UNUSED(index);
  QComboBox *editor = new QComboBox( parent );
  editor->addItems( QStringList() << "Twitter" << "Identi.ca" );
  return editor;
}

void AccountTypeDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString text = index.model()->data( index ).toString();
  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  comboBox->setCurrentIndex( comboBox->findText( text ) );
  return;
}

void AccountTypeDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED(index);
  editor->setGeometry( option.rect );
  return;
}

void AccountTypeDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  model->setData( index, comboBox->currentIndex(), Qt::EditRole );
  return;
}


TwitterAccountsDelegate::TwitterAccountsDelegate( QList<int> checkBoxColumns, QObject *parent ) : QItemDelegate( parent )
{
  this->checkBoxColumns = checkBoxColumns;
}

// TODO: change this, at least remove magic numbers
void TwitterAccountsDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( checkBoxColumns.contains( index.column() ) ) {
    Qt::CheckState state;
    if ( index.model()->data( index, Qt::DisplayRole ).toBool() )
      state = Qt::Checked;
    else
      state = Qt::Unchecked;

    drawDisplay( painter, option, option.rect, " " );
    drawFocus( painter, option, option.rect );
    drawCheck( painter, option, QRect( option.rect.x()+15, option.rect.y()+3 , option.rect.height()-5, option.rect.height()-5 ), state );
  } else {
    QItemDelegate::paint(painter, option, index);
  }
}
