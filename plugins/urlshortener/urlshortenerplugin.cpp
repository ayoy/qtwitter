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
#include <QSettings>
#include <QLineEdit>
#include <QShortcut>
#include <QTimer>

bool TimeoutEventLoop::timeout() const
{
    return m_timeout;
}

int TimeoutEventLoop::exec( QEventLoop::ProcessEventsFlags flags )
{
    m_timeout = false;
    return QEventLoop::exec( flags );
}

void TimeoutEventLoop::quitWithTimeout()
{
    QEventLoop::quit();
    m_timeout = true;
}



UrlShortenerPlugin::UrlShortenerPlugin( QObject *parent ) :
        QObject( parent ),
        urlShortener( new UrlShortener(this) ),
        urlShortenerWidget( new UrlShortenerWidget ),
        shortcut(0),
        m_statusEdit(0)
{
    urlShortenerWidget->setData( urlShortener->shorteners() );
    connect( urlShortener, SIGNAL(shortened(QString,QString)), SLOT(replaceUrl(QString,QString)) );
}

UrlShortenerPlugin::~UrlShortenerPlugin()
{
    delete urlShortenerWidget;
}

QString UrlShortenerPlugin::filterStatusBeforePosting( const QString &status )
{
    if ( !urlShortenerWidget->isAutomatic() )
        return status;

    return shortened( status );
}

void UrlShortenerPlugin::connectToStatusEdit( QLineEdit *statusEdit )
{
    m_statusEdit = statusEdit;
    shortcut = new QShortcut( QKeySequence(), m_statusEdit );
    connect( shortcut, SIGNAL(activated()), SLOT(shortcutActivated()) );
    connect( urlShortenerWidget, SIGNAL(shortcutChanged(QKeySequence)), SLOT(setShortcut(QKeySequence)) );
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
    file->setValue( "shortcut", keySequence );
    file->endGroup();
    file->sync();
}

void UrlShortenerPlugin::loadConfig( QSettings *file )
{
    file->beginGroup( "UrlShortener" );
    urlShortenerWidget->setAutomatic( file->value( "automatic", false ).toBool() );
    urlShortenerWidget->setCurrentIndex( file->value( "shortener", 8 ).toInt() ); // 8 means tr.im - quite reliable
    keySequence = QKeySequence( file->value( "shortcut", QKeySequence() ).value<QKeySequence>() );
    urlShortenerWidget->setShortcut( keySequence.toString( QKeySequence::NativeText ) );
    if ( shortcut )
        shortcut->setKey( keySequence );
    file->endGroup();
    file->sync();
}

void UrlShortenerPlugin::setShortcut( const QKeySequence &seq )
{
    keySequence = seq;
    if ( shortcut )
        shortcut->setKey(seq);
}

void UrlShortenerPlugin::replaceUrl( const QString &oldUrl, const QString &newUrl )
{
    if ( currentStatus && !oldUrl.isEmpty() ) {
        currentStatus->replace( oldUrl, newUrl );
    }
    if ( --requestsCount == 0 )
        emit done();
}

void UrlShortenerPlugin::shortcutActivated()
{
    if ( m_statusEdit )
        m_statusEdit->setText( shortened( m_statusEdit->text() ) );
}

QString UrlShortenerPlugin::shortened( const QString &status )
{
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

        TimeoutEventLoop loop;
        QTimer::singleShot( 10000, &loop, SLOT(quitWithTimeout()) );
        connect( this, SIGNAL(done()), &loop, SLOT(quit()) );
        loop.exec();
    }
    return newStatus;
}

Q_EXPORT_PLUGIN2(UrlShortener, UrlShortenerPlugin);
