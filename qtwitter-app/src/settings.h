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

struct ThemeElement
{
  QString styleSheet;
  QString linkColor;
  QColor listBackgroundColor;
  ThemeElement() :
      styleSheet( QString() ),
      linkColor( QString() ),
      listBackgroundColor( QColor() )
  {}

  ThemeElement( const QString &newStyleSheet, const QString &newLinkColor, const QColor &newListBackgroundColor ) :
      styleSheet( newStyleSheet ),
      linkColor( newLinkColor ),
      listBackgroundColor( newListBackgroundColor )
  {}

  ThemeElement( const ThemeElement &other ) :
      styleSheet( other.styleSheet ),
      linkColor( other.linkColor ),
      listBackgroundColor( other.listBackgroundColor )
  {}
};

struct ThemeData
{
  ThemeElement unread;
  ThemeElement active;
  ThemeElement read;
  ThemeElement disabled;
  ThemeData() :
      unread(),
      active(),
      read(),
      disabled()
  {}

  ThemeData( const ThemeElement &unread, const ThemeElement &active,
             const ThemeElement &read, const ThemeElement &disabled ) :
      unread( unread ),
      active( active ),
      read( read ),
      disabled( disabled )
  {}
};

typedef QPair<QString,ThemeData> ThemeInfo;

class Settings : public QDialog
{
  Q_OBJECT

public:

  static const ThemeInfo STYLESHEET_CARAMEL;
  static const ThemeInfo STYLESHEET_COCOA;
  static const ThemeInfo STYLESHEET_GRAY;
  static const ThemeInfo STYLESHEET_GREEN;
  static const ThemeInfo STYLESHEET_PURPLE;
  static const ThemeInfo STYLESHEET_SKY;

  Settings( MainWindow *mainwinSettings, Core *coreSettings, TwitPicView *twitpicviewSettings, QWidget *parent = 0 );
  ~Settings();

  void loadConfig( bool dialogRejected = false );
  void setProxy();

public slots:
  void saveConfig( int quitting = 0 );
  void show();
  void accept();
  void reject();
  void switchLanguage( int index );

signals:
  void accountsChanged( const QList<Account> &accounts, bool publicTimeline );
  void createAccounts( QWidget *view );

private slots:
  void changeTheme( const QString& );
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
