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


#ifndef STATUSLIST_P_H
#define STATUSLIST_P_H

#include <QObject>
#include "statuslist.h"

class StatusListPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(StatusList);
public:
    StatusListPrivate();
    ~StatusListPrivate();
    void init();
    int addStatus( const Entry &entry );
    void setImageForUrl( const QString &url, QPixmap *pixmap );

    TwitterAPI *twitterapi;
    QList<Status> data;
    bool visible;
    int newStatuses;
    Account *account;
    int active;
    static int maxCount;
    static const int publicMaxCount;

public slots:
    void addEntry( const Entry &entry );
    void addEntries( const EntryList &entries );
    void deleteEntry( quint64 id );
    void setFavorited( quint64 id, bool favorited = true );
    void slotUnauthorized();
    void slotUnauthorized( const QString &status, quint64 inReplyToId );
    void slotUnauthorized( const QString &screenName, const QString &text );
    void slotUnauthorized( quint64 destroyId, Entry::Type type );
    void slotRequestDone( int role );

protected:
    StatusList *q_ptr;
};

#endif // STATUSLIST_P_H
