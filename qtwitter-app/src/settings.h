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

#include <QNetworkProxy>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QAuthenticator>
#include <QSettings>
#include "ui_settings.h"

class QCheckBox;
class QLineEdit;
class QPushButton;
class TwitPicView;
class TweetModel;
class MainWindow;
class Core;
class TwitterAccount;
class TwitterAccountsModel;

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
  ThemeData() :
      unread(),
      active(),
      read()
  {}

  ThemeData( const ThemeElement &_unread, const ThemeElement &_active, const ThemeElement &_read ) :
      unread( _unread ),
      active( _active ),
      read( _read )
  {}
};

typedef QPair<QString,ThemeData> ThemeInfo;

class ConfigFile : public QSettings
{
public:
  static const QString APP_VERSION;
  ConfigFile();
  static QString pwHash( const QString &text );
  void deleteTwitterAccount( int id, int rowCount );
private:
  void convertSettings();
};

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
  void accountsChanged( const QList<TwitterAccount> &accounts, bool publicTimeline );

private slots:
  void fillAccountEditor( const QModelIndex &current, const QModelIndex &previous );
  void addTwitterAccount();
  void deleteTwitterAccount();
  void setTwitterAccountEnabled( bool state );
  void setTwitterAccountLogin( const QString &login );
  void setTwitterAccountPassword( const QString &password );
  void setTwitterAccountDM( bool state );
  void setPublicTimelineEnabled( bool state );
  void changeTheme( const QString& );
  void retranslateUi();
#ifdef Q_WS_X11
  void setBrowser();
#endif

private:
  void applySettings();
  void createLanguageMenu();
  void createUrlShortenerMenu();
  QTranslator translator;
  QNetworkProxy proxy;
  MainWindow *mainWindow;
  Core *core;
  TwitterAccountsModel *accountsModel;
  TwitPicView *twitPicView;
  QMap<QString,ThemeData> themes;
  Ui::Settings ui;
#ifdef Q_WS_X11
  QCheckBox *useCustomBrowserCheckBox;
  QLineEdit *selectBrowserEdit;
  QPushButton *selectBrowserButton;
#endif
};

#endif //SETTINGS_H
