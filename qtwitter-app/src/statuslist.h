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


#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QObject>
#include <QList>
#include <QDataStream>
#include <twitterapi/twitterapi.h>
#include "statusmodel.h"
#include "statuswidget.h"

class QPixmap;
struct Account;

struct Status {
    Entry entry;
    StatusModel::StatusState state;
    QPixmap image;
    bool operator==( const Status &other )
    {
        return ( entry == other.entry
                 && state == other.state
                 && image.cacheKey() == other.image.cacheKey() );
    }
};

QDataStream& operator<<( QDataStream & out, const Status &status );
QDataStream& operator>>( QDataStream & in, Status &status );

Q_DECLARE_METATYPE(Status)

        class StatusListPrivate;

class StatusList : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString serviceUrl READ serviceUrl );
    Q_PROPERTY( QString login READ login );
    Q_PROPERTY( bool dm READ dm );
    Q_PROPERTY( bool visible READ isVisible WRITE setVisible );
    // index of the active status
    Q_PROPERTY( int active READ active WRITE setActive );
    
public:
    StatusList( Account *account, QObject *parent );
    ~StatusList();

    int newStatusesCount();
    void markAllAsRead();

    bool remove( int from, int count );
    static void setMaxCount( int maxCount );

    // status list accessors
    bool dm() const;
    QString serviceUrl() const;
    QString login() const;
    void setVisible( bool visible );
    bool isVisible() const;
    void setData( int index, const Status &status );
    const Status& data( int index ) const;
    void setState( int index, StatusModel::StatusState state );
    StatusModel::StatusState state( int index ) const;
    void setImage( int index, const QPixmap &pixmap );
    const QList<Status>& getData() const;
    void setStatuses( const QList<Status> &statuses );
    int active() const;
    void setActive( int active );
    // end of accessors

    int size() const;


public slots:
    void requestFriendsTimeline();
    void requestMentions();
    void requestDirectMessages();
    void requestNewStatus( const QString &status, quint64 inReplyTo = 0 );
    void requestNewDM( const QString &screenName, const QString &text );
    void postDMDialog( const QString &screenName );
    void requestDestroy( quint64 id, Entry::Type type );
    void requestCreateFavorite( quint64 id );
    void requestDestroyFavorite( quint64 id );

    void slotDirectMessagesChanged( bool isEnabled );

signals:
    void statusAdded( int index );
    void statusDeleted( int index );
    void dataChanged( int index );
    void stateChanged( int index );
    void favoriteChanged( int index );
    void imageChanged( int index );

protected:
    StatusListPrivate * const d_ptr;

private:
    Q_DECLARE_PRIVATE(StatusList);
};

#endif // STATUSLIST_H
