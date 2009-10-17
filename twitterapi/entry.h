/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef ENTRY_H
#define ENTRY_H

#include <QMetaType>
#include <QDateTime>
#include <QDataStream>
#include "twitterapi_global.h"
#include "userinfo.h"


struct TWITTERAPI_EXPORT Entry
{
    enum Type {
        Status,
        DirectMessage
    };

    Entry( Entry::Type entryType = Entry::Status );

    void initialize();
    bool checkContents();

    bool operator == ( const Entry &other );

    Type type;
    bool isOwn;
    quint64 id;
    QString text;
    QString originalText;
    QDateTime timestamp;
    QDateTime localTime;
    bool hasInReplyToStatusId;
    quint64 inReplyToStatusId;
    QString inReplyToScreenName;
    bool favorited;
    UserInfo userInfo;
};

Q_DECLARE_METATYPE(Entry);


TWITTERAPI_EXPORT QDataStream& operator<<( QDataStream &out, const Entry &entry );
TWITTERAPI_EXPORT QDataStream& operator>>( QDataStream &in, Entry &entry );


#endif // ENTRY_H
