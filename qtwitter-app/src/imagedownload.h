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


#ifndef IMAGEDOWNLOAD_H
#define IMAGEDOWNLOAD_H

#include <QObject>
#include <QMap>
#include <QCache>
#include <QPixmap>
#include <QNetworkAccessManager>


class ImageDownload : public QObject
{
    Q_OBJECT

public:
    static ImageDownload* instance();
    virtual ~ImageDownload();

    void imageGet( const QString& imageUrl );

protected:
    ImageDownload( QObject *parent = 0 );

signals:
    void imageReadyForUrl( const QString& path );

private slots:
    void requestFinished( QNetworkReply *reply );

private:
    QMap<QString,QNetworkAccessManager*> connections;
    static ImageDownload *m_inst;
};

#endif //IMAGEDOWNLOAD_H
