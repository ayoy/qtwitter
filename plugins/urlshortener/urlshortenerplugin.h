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


#ifndef URLSHORTENERPLUGIN_H
#define URLSHORTENERPLUGIN_H

#include <QObject>
#include <plugininterfaces.h>

class UrlShortener;
class UrlShortenerWidget;

class UrlShortenerPlugin : public QObject,
                           public StatusFilterInterface,
                           public SettingsTabInterface,
                           public ConfigFileInterface
{
    Q_OBJECT
    Q_INTERFACES(StatusFilterInterface SettingsTabInterface ConfigFileInterface);

public:
    UrlShortenerPlugin( QObject *parent = 0 );
    virtual ~UrlShortenerPlugin();

    // StatusFilterInterface
    QString filterStatus( const QString &status );

    // SettingsTabInterface
    QString tabName();
    QWidget *settingsWidget();

    // ConfigFileInterface
    void saveConfig( QSettings *file );
    void loadConfig( QSettings *file );

signals:
    void done();

private slots:
    void replaceUrl( const QString &oldUrl, const QString &newUrl );

private:
    UrlShortener *urlShortener;
    UrlShortenerWidget *urlShortenerWidget;
    QString *currentStatus;
    int requestsCount;

};

#endif // URLSHORTENERPLUGIN_H
