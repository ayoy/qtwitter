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


#ifndef PLUGININTERFACES_H
#define PLUGININTERFACES_H

#include <QtPlugin>

class QString;
class QSettings;
class QLineEdit;

class StatusFilterInterface
{
public:
    virtual ~StatusFilterInterface() {}

    virtual QString filterStatusBeforePosting( const QString &status ) = 0;
    virtual void connectToStatusEdit( QLineEdit *statusEdit ) = 0;
};

class SettingsTabInterface
{
public:
    virtual ~SettingsTabInterface() {}

    virtual QString tabName() = 0;
    virtual QWidget* settingsWidget() = 0;
};

class ConfigFileInterface
{
public:
    virtual ~ConfigFileInterface() {}

    virtual void saveConfig( QSettings *file ) = 0;
    virtual void loadConfig( QSettings *file ) = 0;
};

Q_DECLARE_INTERFACE(StatusFilterInterface, "net.ayoy.qTwitter.StatusFilterInterface/1.0");
Q_DECLARE_INTERFACE(SettingsTabInterface, "net.ayoy.qTwitter.SettingsTabInterface/1.0");
Q_DECLARE_INTERFACE(ConfigFileInterface, "net.ayoy.qTwitter.ConfigFileInterface/1.0");

#endif // PLUGININTERFACES_H
