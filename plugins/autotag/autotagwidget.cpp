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


#include "autotagwidget.h"
#include "ui_autotagwidget.h"

#include <QStringListModel>
#include <QModelIndexList>
#include <QRegExp>
#include <QRegExpValidator>
#include <QKeyEvent>

AutoTagWidget::AutoTagWidget( QWidget *parent ) :
        QWidget( parent ),
        m_ui( new Ui::AutoTagWidget ),
        tagsModel( new QStringListModel( this ) )
{
    m_ui->setupUi( this );
    m_ui->viewWords->setModel( tagsModel );
    connect( m_ui->btnAdd, SIGNAL(clicked()), SLOT(addTags()) );
    connect( m_ui->btnDelete, SIGNAL(clicked()), SLOT(deleteTag()) );

    connect( m_ui->lineWord, SIGNAL(textChanged(QString)), SLOT(updateAddButton()) );

    connect( m_ui->viewWords->selectionModel(),
             SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             SLOT(updateDeleteButton()) );

    m_ui->lineWord->installEventFilter( this );
    m_ui->lineWord->setValidator( new QRegExpValidator( QRegExp("[a-zA-Z0-9-_\\.,; ]*"), this ) );
}

AutoTagWidget::~AutoTagWidget()
{
    delete m_ui;
    m_ui = 0;
}

bool AutoTagWidget::isActive() const
{
    return m_ui->chbEnable->isChecked();
}

void AutoTagWidget::setActive( bool active )
{
    m_ui->chbEnable->setChecked( active );
}

QStringList AutoTagWidget::tags() const
{
    return tagsModel->stringList();
}

void AutoTagWidget::setTags( const QStringList &tags )
{
    tagsModel->setStringList( tags );
}

void AutoTagWidget::addTags()
{
    QString input = m_ui->lineWord->text();
    QStringList list = input.split( QRegExp( "([,;]|[,;]? +| +)"), QString::SkipEmptyParts );
    list.append( tagsModel->stringList() );
    list.removeDuplicates();
    list.sort();
    tagsModel->setStringList( list );
    m_ui->lineWord->clear();
}

void AutoTagWidget::deleteTag()
{
    QModelIndexList indexes = m_ui->viewWords->selectionModel()->selection().indexes();
    QList<int> rowsList;
    foreach( const QModelIndex &index, indexes ) {
        rowsList << index.row();
    }
    qSort(rowsList);
    QList<int>::Iterator i;
    for ( i = rowsList.end(); i != rowsList.begin(); --i ) {
        tagsModel->removeRow( *i );
    }
    tagsModel->removeRow( *i );
}

void AutoTagWidget::changeEvent( QEvent *event )
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:;
    }
}

bool AutoTagWidget::eventFilter( QObject *watched, QEvent *event )
{
    Q_UNUSED(watched);

    if ( event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ( keyEvent->key() == Qt::Key_Enter ||
             keyEvent->key() == Qt::Key_Return ) {
            addTags();
            return true;
        }
    }
    return false;
}

void AutoTagWidget::updateAddButton()
{
    bool enabled = !m_ui->lineWord->text().isEmpty();
    m_ui->btnAdd->setEnabled( enabled );
}

void AutoTagWidget::updateDeleteButton()
{
    bool enabled = !m_ui->viewWords->selectionModel()->selection().isEmpty();
    m_ui->btnDelete->setEnabled( enabled );
}
