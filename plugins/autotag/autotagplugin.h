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


#ifndef AUTOTAGPLUGIN_H
#define AUTOTAGPLUGIN_H

#include <QObject>
#include <plugininterfaces.h>

class AutoTagPlugin : public QObject,
                      public StatusFilterInterface,
                      public SettingsTabInterface
{
    Q_OBJECT
    Q_INTERFACES(StatusFilterInterface);
    Q_INTERFACES(SettingsTabInterface);

public:
    AutoTagPlugin( QObject *parent = 0 );
    virtual ~AutoTagPlugin();

    // StatusFilterInterface
    QString filterStatus( const QString &status );

    // SettingsTabInterface
    QString tabName();
    QWidget *settingsWidget();

private:
    QWidget *autoTagWidget;

};

#endif // AUTOTAGPLUGIN_H
