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


#include "qtwitterapp.h"

#include "core.h"
#include "mainwindow.h"
#include "settings.h"
#include "statusmodel.h"
#include "twitpicview.h"
#include <account.h>

#include <QSignalMapper>

QTwitterApp::QTwitterApp( int & argc, char **argv ) :
        QApplication( argc, argv ),
        m_mainWindow(0),
        m_twitPic(0)
{
    m_core = new Core( this );
    m_settingsDialog = new Settings( m_core );

    QSignalMapper *mapper = new QSignalMapper( this );
    mapper->setMapping( qApp, 1 );
    connect( this, SIGNAL(aboutToQuit()), mapper, SLOT(map()) );
    connect( mapper, SIGNAL(mapped(int)), m_settingsDialog, SLOT(saveConfig(int)) );
}

QTwitterApp::~QTwitterApp()
{
    delete m_settingsDialog;
}

QTwitterApp* QTwitterApp::instance()
{
    return static_cast<QTwitterApp*>( QApplication::instance() );
}

Core* QTwitterApp::core()
{
    return instance()->m_core;
}

MainWindow* QTwitterApp::mainWindow()
{
    return instance()->m_mainWindow;
}

Settings* QTwitterApp::settingsDialog()
{
    return instance()->m_settingsDialog;
}

void QTwitterApp::openSettings()
{
    m_settingsDialog->show();
}

void QTwitterApp::openTwitPic()
{
    m_twitPic = new TwitPicView( m_mainWindow );
    connect( m_twitPic, SIGNAL(uploadPhoto(QString,QString,QString)), m_core, SLOT(uploadPhoto(QString,QString,QString)) );
    connect( m_twitPic, SIGNAL(abortUpload()), m_core, SLOT(abortUploadPhoto()) );
    connect( m_core, SIGNAL(twitPicResponseReceived()), m_twitPic, SLOT(resetForm()) );
    connect( m_core, SIGNAL(twitPicDataSendProgress(qint64,qint64)), m_twitPic, SLOT(showUploadProgress(qint64,qint64)) );
    connect( m_core, SIGNAL(accountsUpdated(QList<Account>)), m_twitPic, SLOT(setupAccounts(QList<Account>)) );
    m_twitPic->setupAccounts( m_core->twitpicLogins() );
    m_twitPic->setAttribute( Qt::WA_DeleteOnClose, true );
    m_twitPic->show();
}

void QTwitterApp::loadConfig()
{
    m_settingsDialog->loadConfig();
}

void QTwitterApp::registerMainWindow( MainWindow *mainWindow )
{
    if ( !instance()->m_mainWindow ) {
        instance()->m_mainWindow = mainWindow;
        instance()->m_core->restoreSession();
    }
}

void QTwitterApp::unregisterMainWindow( MainWindow *mainWindow )
{
    if ( instance()->m_mainWindow == mainWindow ) {
        instance()->m_mainWindow = 0;
        instance()->m_core->storeSession();
    }
}
