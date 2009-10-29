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


#ifndef PLUGINMANAGERWIDGET_H
#define PLUGINMANAGERWIDGET_H

#include <QWidget>

namespace Ui {
    class PluginManagerWidget;
}

class QStandardItemModel;
class QModelIndex;

class PluginManagerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QString tabName READ tabName );

public:
    explicit PluginManagerWidget( QWidget *parent = 0 );
    virtual ~PluginManagerWidget();

    QString tabName() const;
    void registerPluginWidget( const QString &name, QWidget *widget );

protected:
    void changeEvent( QEvent *event );

private slots:
    void selectPlugin( const QModelIndex &index );

private:
    Ui::PluginManagerWidget *m_ui;

    QStandardItemModel *pluginsModel;
};

#endif // PLUGINMANAGERWIDGET_H
