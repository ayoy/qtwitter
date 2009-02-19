/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

class TweetModel;
class MainWindow;
class LoopedSignal;
class Core;

struct ThemeData {
  QString styleSheet;
  QString linkColor;
  QColor listBackgroundColor;
  ThemeData() :
      styleSheet( QString() ),
      linkColor( QString() ),
      listBackgroundColor( QColor() )
  {}
  ThemeData( const QString &newStyleSheet, const QString &newLinkColor, const QColor &newListBackgroundColor ) :
      styleSheet( newStyleSheet ),
      linkColor( newLinkColor ),
      listBackgroundColor( newListBackgroundColor )
  {}
};

class Settings : public QDialog
{
  Q_OBJECT

public:
  static const ThemeData STYLESHEET_CARAMEL;
  static const ThemeData STYLESHEET_SKY;
//  static const QMap<QString,StyleSheetData> STYLESHEETS;

  Settings( TweetModel *tweetModel, MainWindow *mainwinSettings, LoopedSignal *loopSettings, Core *coreSettings, QWidget *parent = 0 );
  ~Settings();
  bool createConfigFile();
  void loadConfig( bool dialogRejected = false );
  QDir directoryOf( const QString& );
  void setProxy();

public slots:
  void accept();
  void reject();
  void switchLanguage( int );
  void setAuthDataInDialog( const QAuthenticator& );
  void switchToPublic();
  void saveConfig();
  void show();


private slots:
  void retranslateUi();
  void changeTheme( const QString& );
#ifdef Q_WS_X11
  void setBrowser();
#endif

signals:
  void settingsOK();
  void languageChanged();

private:
  void applySettings();
  void createLanguageMenu();
  QTranslator translator;
  QFile configFile;
  QNetworkProxy proxy;
  Ui::Settings ui;
#ifdef Q_WS_X11
  QLineEdit *selectBrowserEdit;
  QLabel *selectBrowserLabel;
  QPushButton *selectBrowserButton;
#endif
  TweetModel *model;
  MainWindow *mainWindow;
  LoopedSignal *loopedSignal;
  Core *core;
};

#endif //SETTINGS_H
