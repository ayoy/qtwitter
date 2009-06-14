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

class QFile;
class QDir;
class QAuthenticator;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QModelIndex;
class TwitPicView;
class StatusModel;
class MainWindow;
class Core;
class Account;
class AccountsModel;
class AccountsController;


class Settings : public QDialog
{
  Q_OBJECT

public:

  Settings( MainWindow *mainwinSettings, Core *coreSettings, TwitPicView *twitpicviewSettings, QWidget *parent = 0 );
  ~Settings();

  void loadConfig( bool dialogRejected = false );
  void setProxy();

public slots:
  void saveConfig( int quitting = 0 );
  void checkForUpdate();
  void show();
  void accept();
  void reject();
  void switchLanguage( int index );

signals:
  void accountsChanged( const QList<Account> &accounts, bool publicTimeline );
  void createAccounts( QWidget *view );

private slots:
  void changeTheme( const QString& );
  void readUpdateReply( bool available, const QString &version );
  void retranslateUi();
#ifdef Q_WS_X11
  void setBrowser();
#endif

private:
  void applySettings();
  void createLanguageMenu();
  void createUrlShortenerMenu();
  bool updateAccountsOnExit;
  MainWindow *mainWindow;
  Core *core;
  TwitPicView *twitPicView;
  QMap<QString,ThemeData> themes;
  Ui::Settings ui;
  QTranslator translator;
#ifdef Q_WS_X11
  QCheckBox *useCustomBrowserCheckBox;
  QLineEdit *selectBrowserEdit;
  QPushButton *selectBrowserButton;
#endif
};

#endif //SETTINGS_H
