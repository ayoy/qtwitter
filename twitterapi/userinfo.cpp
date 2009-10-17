/***************************************************************************
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




#include "userinfo.h"



UserInfo::UserInfo() :
        id(-1),
        name(QString()),
        screenName(QString()),
        location(QString()),
        description(QString()),
        homepage(QString()),
        hasHomepage(false),
        profileProtected(false),
        followersCount(-1),
        friendsCount(-1),
        utcOffset(-1),
        statusesCount(-1)
{
}


void UserInfo::initialize()
{
    id = -1;
    name = QString();
    screenName = QString();
    homepage = QString();
    hasHomepage = false;
    profileProtected = false;
    imageUrl = QString();
    location = QString();
    description = QString();
    followersCount = -1;
    friendsCount = -1;
    utcOffset = -1;
    statusesCount = -1;
}

bool UserInfo::checkContents()
{
    if( !hasHomepage)
        homepage = QString();

    if( (id != -1) &&
        !name.isNull() &&
        !screenName.isNull() &&
        hasHomepage ? !homepage.isNull() : true &&
        friendsCount != -1 &&
        followersCount != -1 &&
        utcOffset != -1)
        return true;

    return false;
}

QDataStream& operator<<( QDataStream & out, const UserInfo &userInfo )
{
    out << userInfo.id;
    out << userInfo.name;
    out << userInfo.screenName;
    out << userInfo.location;
    out << userInfo.description;
    out << userInfo.imageUrl;
    out << userInfo.homepage;
    out << userInfo.hasHomepage;
    out << userInfo.profileProtected;
    out << userInfo.followersCount;
    out << userInfo.friendsCount;
    out << userInfo.utcOffset;
    out << userInfo.statusesCount;
    return out;
}

QDataStream& operator>>( QDataStream & in, UserInfo &userInfo )
{
    in >> userInfo.id;
    in >> userInfo.name;
    in >> userInfo.screenName;
    in >> userInfo.location;
    in >> userInfo.description;
    in >> userInfo.imageUrl;
    in >> userInfo.homepage;
    in >> userInfo.hasHomepage;
    in >> userInfo.profileProtected;
    in >> userInfo.followersCount;
    in >> userInfo.friendsCount;
    in >> userInfo.utcOffset;
    in >> userInfo.statusesCount;
    return in;
}
