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


#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include <QList>
#include <QDataStream>
#include "statusmodel.h"

class QPixmap;

struct Status {
    enum State {
        Disabled,
        Unread,
        Read,
        Active
    };

    Entry entry;
    State state;
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

#endif // STATUS_H
