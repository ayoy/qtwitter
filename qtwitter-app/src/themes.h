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


#ifndef THEMES_H
#define THEMES_H

#include <QPair>
#include <QColor>

struct ThemeElement
{
    QString styleSheet;
    QString linkColor;
    ThemeElement() :
            styleSheet( QString() ),
            linkColor( QString() )
    {}

    ThemeElement( const QString &newStyleSheet, const QString &newLinkColor ) :
            styleSheet( newStyleSheet ),
            linkColor( newLinkColor )
    {}

    ThemeElement( const ThemeElement &other ) :
            styleSheet( other.styleSheet ),
            linkColor( other.linkColor )
    {}
};

struct ThemeData
{
    ThemeElement unread;
    ThemeElement active;
    ThemeElement read;
    ThemeElement disabled;
    QColor listBackgroundColor;
    ThemeData() :
            unread(),
            active(),
            read(),
            disabled(),
            listBackgroundColor()
    {}

    ThemeData( const ThemeElement &unread, const ThemeElement &active,
               const ThemeElement &read, const ThemeElement &disabled,
               const QColor &listBackgroundColor ) :
    unread( unread ),
    active( active ),
    read( read ),
    disabled( disabled ),
    listBackgroundColor( listBackgroundColor )
    {}
};

typedef QPair<QString,ThemeData> ThemeInfo;

struct Themes {
    static const ThemeInfo STYLESHEET_COCOA;
    static const ThemeInfo STYLESHEET_GRAY;
    static const ThemeInfo STYLESHEET_GREEN;
    static const ThemeInfo STYLESHEET_PURPLE;
    static const ThemeInfo STYLESHEET_SKY;
};


#endif // THEMES_H
