/***************************************************************************
 *   Copyright (C) 2009 by Dominik Kapusta            <d@ayoy.net>         *
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


#ifndef URLSHORTENER_H
#define URLSHORTENER_H

#include <QObject>
#include <QMap>

class QNetworkReply;
class QNetworkAccessManager;


class UrlShortenerImplementation;

class UrlShortener : public QObject
{
    Q_OBJECT

public:
    enum Shortener {
        ShortenerIsgd,
        ShortenerTrim,
        ShortenerMetamark,
        ShortenerTinyurl,
        ShortenerTinyarrows,
        ShortenerUnu,
        ShortenerBitly,
        ShortenerDigg,
        ShortenerMigreme,
        ShortenerBooom
    };

    UrlShortener( QObject *parent = 0 );
    void shorten( const QString &url, Shortener shorteningService = ShortenerIsgd );

    QMap<QString,int> shorteners() const;

signals:
    void shortened( const QString &oldUrl, const QString &newUrl );
    void errorMessage( const QString &message );

private:
    UrlShortenerImplementation *shortenerInstance;
};


#endif // URLSHORTENER_H
