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

class AutoTagWidget;

class AutoTagPlugin : public QObject,
                      public StatusFilterInterface,
                      public SettingsTabInterface,
                      public ConfigFileInterface
{
    Q_OBJECT
    Q_INTERFACES(StatusFilterInterface SettingsTabInterface ConfigFileInterface);

public:
    AutoTagPlugin( QObject *parent = 0 );
    virtual ~AutoTagPlugin();

    // StatusFilterInterface
    QString filterStatusBeforePosting( const QString &status );
    void connectToStatusEdit( QLineEdit *statusEdit ) { Q_UNUSED(statusEdit); }

    // SettingsTabInterface
    QString tabName();
    QWidget *settingsWidget();

    // ConfigFileInterface
    void saveConfig( QSettings *file );
    void loadConfig( QSettings *file );

private:
    AutoTagWidget *autoTagWidget;

};

#endif // AUTOTAGPLUGIN_H
