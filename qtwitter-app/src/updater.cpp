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


#include "updater.h"
#include "configfile.h"

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QDebug>

Updater::Updater( QObject *parent ) :
        QNetworkAccessManager( parent )
{
    connect( this, SIGNAL(finished(QNetworkReply*)), SLOT(readReply(QNetworkReply*)) );
}

Updater::~Updater()
{
}

void Updater::checkForUpdate()
{
    QByteArray data( "version=" );
    data.append( ConfigFile::APP_VERSION );
    qDebug() << "checking for updates: current version is" << ConfigFile::APP_VERSION;
    post( QNetworkRequest( QUrl( "http://qtwitter.ayoy.net/cgi-bin/latest2" ) ), data );
}

void Updater::readReply( QNetworkReply *reply )
{
    int returnCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    qDebug() << returnCode;
    if ( returnCode == 200 ) {
        QString latest = reply->readLine();
        latest.chop(1); // remove the trailing enter
        QString changes = reply->readAll();
        bool available = !latest.isEmpty();
        emit updateChecked( available, available ? latest : ConfigFile::APP_VERSION, available ? changes : QString() );
    }
}
