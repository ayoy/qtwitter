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

class MainWindow;
class LoopedSignal;
class Core;

class Settings : public QDialog
{
  Q_OBJECT

public:
  Settings( MainWindow *mainwinSettings, LoopedSignal *loopSettings, Core *coreSettings, QWidget *parent = 0 );
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
signals:
  void settingsOK();

private:
  void applySettings();
  void createLanguageMenu();
  void retranslateUi();
  QTranslator translator;
  QFile configFile;
  QNetworkProxy proxy;
  Ui::Settings ui;
  MainWindow *mainWindow;
  LoopedSignal *loopedSignal;
  Core *core;
};

#endif //SETTINGS_H
