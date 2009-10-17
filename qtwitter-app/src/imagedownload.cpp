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


#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include "imagedownload.h"

ImageDownload* ImageDownload::m_inst = 0;

ImageDownload* ImageDownload::instance()
{
    if ( !m_inst ) {
        m_inst = new ImageDownload( qApp );
    }
    return m_inst;
}

ImageDownload::ImageDownload( QObject *parent ) :
        QObject( parent )
{
    imageCache.setMaxCost( 50 );
}

ImageDownload::~ImageDownload()
{
    m_inst = 0;
}

void ImageDownload::imageGet( const QString &imageUrl )
{
    if ( imageCache.contains( imageUrl ) ) {
        emit imageReadyForUrl( imageUrl, imageCache[ imageUrl ] );
        return;
    }

    QUrl url( imageUrl );
    QString host( url.host() );
    QNetworkRequest request( url );

    if ( !connections.contains( host ) ) {
        connections.insert( host, new QNetworkAccessManager( this ) );
        connect( connections[ host ], SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
    }

    connections[ host ]->get( request );
    imageCache.insert( imageUrl, new QPixmap );
}

bool ImageDownload::contains( const QString &imageUrl ) const
{
    return imageCache.contains( imageUrl );
}

QPixmap* ImageDownload::imageFromUrl( const QString &imageUrl ) const
{
    return imageCache[ imageUrl ];
}

void ImageDownload::requestFinished( QNetworkReply *reply )
{
    int replyCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

    if ( replyCode != 200 )
        return;

    QPixmap *pixmap = new QPixmap;
    pixmap->loadFromData( reply->readAll() );
    imageCache.insert( reply->url().toString(), pixmap );
    emit imageReadyForUrl( reply->url().toString(), pixmap );
}


/*! \class ImageDownload
    \brief A class for downlading images for Statuses.

    This is a class that provides interface for downloading user profile images
    for Statuses.
*/

/*! \fn ImageDownload::ImageDownload()
    A default constructor.
*/

/*! \fn void ImageDownload::imageGet( Entry *entry )
    This method invokes HttpConnection::prepareRequest() for a path
    given in \a entry, assigns allocated data to newly created
    ImageData instance and then issues QHttp::get().
    \param entry Entry for which the image will be downloaded.
*/

/*! \fn void ImageDownload::clearData()
    Forces clearing of temporary images store, gathered while updating timeline.
*/

/*! \fn void ImageDownload::imageReadyForUrl( const QString& path, QPixmap image )
    Emitted when the request is finished and the image is ready to be displayed.
    \param path The image's URL.
    \param image The downloaded image.
*/
