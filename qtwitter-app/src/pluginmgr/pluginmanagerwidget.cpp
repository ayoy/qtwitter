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


#include "pluginmanagerwidget.h"
#include "ui_pluginmanagerwidget.h"

#include <QStandardItemModel>

PluginManagerWidget::PluginManagerWidget( QWidget *parent ) :
        QWidget( parent ),
        m_ui( new Ui::PluginManagerWidget )
{
    m_ui->setupUi(this);
    pluginsModel = new QStandardItemModel(this);
    m_ui->viewPlugins->setModel( pluginsModel );
    connect( m_ui->viewPlugins, SIGNAL(clicked(QModelIndex)), SLOT(selectPlugin(QModelIndex)) );
}

PluginManagerWidget::~PluginManagerWidget()
{
    delete m_ui;
    m_ui = 0;
}

QString PluginManagerWidget::tabName() const
{
    return tr( "Plugins" );
}

void PluginManagerWidget::registerPluginWidget( const QString &name, QWidget *widget )
{
    int index = m_ui->pluginStackedWidget->addWidget(widget);

    QStandardItem *item = new QStandardItem( name );
    item->setData( index );

    pluginsModel->appendRow( item );
    if ( pluginsModel->rowCount() == 1 ) {
        m_ui->viewPlugins->setCurrentIndex( pluginsModel->index(0,0) );
    }
}

void PluginManagerWidget::changeEvent( QEvent *event )
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
        break;
    default:;
    }
}

void PluginManagerWidget::selectPlugin( const QModelIndex &index )
{
    QStandardItem *item = pluginsModel->item( index.row() );
    int widgetIndex = item->data().toInt();

    m_ui->pluginStackedWidget->setCurrentIndex( widgetIndex );
}
