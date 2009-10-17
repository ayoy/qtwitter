/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#ifndef TWITPICENGINE_H
#define TWITPICENGINE_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class Core;

class TwitPicEngine : public QObject
{
    Q_OBJECT

public:
    TwitPicEngine( Core *coreParent, QObject *parent = 0 );
    ~TwitPicEngine();

    void postContent( const QString &login, const QString &password, QString photoPath, QString status );

public slots:
    void abort();

signals:
    void finished();
    void errorMessage( const QString &message );
    void completed( bool responseStatus, QString message, bool newStatus );

private slots:
    void readReply( QNetworkReply *reply );

private:
    void parseReply( const QByteArray &reply );

private:
    enum ErrorId {
        ErrInvalidLogin = 1001,
        ErrImageNotFound = 1002,
        ErrInvalidType = 1003,
        ErrOversized = 1004
    };

    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    Core *coreParent;
};

#endif // TWITPICENGINE_H
