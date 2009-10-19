/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#include "urlshortenerplugin.h"
#include "urlshortener.h"
#include "urlshortenerwidget.h"

#include <QRegExp>
#include <QEventLoop>
#include <QSettings>

UrlShortenerPlugin::UrlShortenerPlugin( QObject *parent ) :
        QObject( parent ),
        urlShortener( new UrlShortener(this) ),
        urlShortenerWidget( new UrlShortenerWidget )
{
    urlShortenerWidget->setData( urlShortener->shorteners() );
    connect( urlShortener, SIGNAL(shortened(QString,QString)), SLOT(replaceUrl(QString,QString)) );
}

UrlShortenerPlugin::~UrlShortenerPlugin()
{
    delete urlShortenerWidget;
}

QString UrlShortenerPlugin::filterStatus( const QString &status )
{
    if ( !urlShortenerWidget->isAutomatic() )
        return status;

    QRegExp rx( "((ftp|http|https)://(\\w+:{0,1}\\w*@)?([^ ]+)(:[0-9]+)?(/|/([\\w#!:.?+=&%@!-/]))?)",
                Qt::CaseInsensitive );

    QString newStatus = status;
    QStringList urls;

    int pos = 0;
    while ( (pos = rx.indexIn(newStatus, pos)) != -1 ) {
        urls << rx.cap(1);
        pos += rx.matchedLength();
    }
    urls.removeDuplicates();

    if ( !urls.isEmpty() ) {
        requestsCount = 0;
        currentStatus = &newStatus;

        foreach( const QString &url, urls ) {
            urlShortener->shorten( url, (UrlShortener::Shortener) urlShortenerWidget->currentShortener() );
            requestsCount++;
        }

        QEventLoop loop;
        connect( this, SIGNAL(done()), &loop, SLOT(quit()) );
        loop.exec();
    }
    return newStatus;
}

void UrlShortenerPlugin::replaceUrl( const QString &oldUrl, const QString &newUrl )
{
//    qDebug() << __PRETTY_FUNCTION__ << oldUrl << newUrl;
    if ( currentStatus && !oldUrl.isEmpty() ) {
        currentStatus->replace( oldUrl, newUrl );
    }
    if ( --requestsCount == 0 )
        emit done();
}

QString UrlShortenerPlugin::tabName()
{
    return tr( "Url shortening" );
}

QWidget* UrlShortenerPlugin::settingsWidget()
{
    return urlShortenerWidget;
}

void UrlShortenerPlugin::saveConfig( QSettings *file )
{
    file->beginGroup( "UrlShortener" );
    file->setValue( "automatic", urlShortenerWidget->isAutomatic() );
    file->setValue( "shortener", urlShortenerWidget->currentIndex() );
    file->endGroup();
    file->sync();
}

void UrlShortenerPlugin::loadConfig( QSettings *file )
{
    file->beginGroup( "UrlShortener" );
    urlShortenerWidget->setAutomatic( file->value( "automatic", false ).toBool() );
    urlShortenerWidget->setCurrentIndex( file->value( "shortener", 8 ).toInt() ); // 8 means tr.im - quite reliable
    file->endGroup();
    file->sync();
}

Q_EXPORT_PLUGIN2(UrlShortener, UrlShortenerPlugin);
