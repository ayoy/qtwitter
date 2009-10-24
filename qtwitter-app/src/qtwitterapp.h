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


#ifndef QTWITTERAPP_H
#define QTWITTERAPP_H

#include <QApplication>

class Core;
class MainWindow;
class TwitPicView;
class Settings;

class QTwitterApp : public QApplication
{
    Q_OBJECT
public:
    static QTwitterApp* instance();

    QTwitterApp( int & argc, char **argv );
    virtual ~QTwitterApp();

    static Core* core();
    static MainWindow* mainWindow();
    static Settings* settingsDialog();

public slots:
    void openSettings();
    void openTwitPic();
    void loadConfig();

private:
    static void registerMainWindow( MainWindow *mainWindow );
    static void unregisterMainWindow( MainWindow *mainWindow );

    Core *m_core;
    MainWindow *m_mainWindow;
    TwitPicView *m_twitPic;
    Settings *m_settingsDialog;

    friend class MainWindow;
};

#endif // QTWITTERAPP_H
