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


#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>
#include <QTranslator>
#include "themes.h"
#include "ui_settings.h"

class QCheckBox;
class QLineEdit;
class QPushButton;
class TwitPicView;
class StatusModel;
class MainWindow;
class Core;
class Account;
class AccountsModel;
class AccountsController;
class ConfigFileInterface;
class PluginManagerWidget;

class Settings : public QDialog
{
    Q_OBJECT

public:

    Settings( Core *coreSettings, QWidget *parent = 0 );
    ~Settings();

    void loadConfig( bool dialogRejected = false );
    void setProxy();
    void addTab( const QString &tabName, QWidget *tabWidget );
    void addConfigFilePlugin( ConfigFileInterface *iface );

public slots:
    void saveConfig( int quitting = 0 );
    void checkForUpdate();
    void show();
    void accept();
    void reject();
    void switchLanguage( int index );

signals:
    void createAccounts( QWidget *view );

protected:
    void changeEvent( QEvent *e );

private slots:
    void changeTheme( const QString& );
    void readUpdateReply( bool available, const QString &version, const QString &changes );
    void retranslateUi();
#ifdef Q_WS_X11
    void setBrowser();
#endif

private:
    void applySettings();
    void createLanguageMenu();
    bool updateAccountsOnExit;
    Core *core;
    QMap<QString,ThemeData> themes;
    Ui::Settings ui;
    QTranslator translator;
#ifdef Q_WS_X11
    QCheckBox *useCustomBrowserCheckBox;
    QLineEdit *selectBrowserEdit;
    QPushButton *selectBrowserButton;
#endif
    PluginManagerWidget *pluginManagerTab;
    QList<ConfigFileInterface*> configFilePlugins;
};

#endif //SETTINGS_H
