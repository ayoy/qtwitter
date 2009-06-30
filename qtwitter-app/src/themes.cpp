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


#include "themes.h"

const ThemeInfo Themes::STYLESHEET_COCOA   = ThemeInfo( QString( "Cocoa" ),
                                                        // unread
                                                        ThemeData( ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(184, 133, 92);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(154, 103, 72);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(21, 21, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(51, 51, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(36, 36, 35); }"
                                                                                          "a:visited { color: rgb(36, 36, 35); }" ) ),
                                                                   // active
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(224, 163, 102);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(224, 183, 82);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(51, 51, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(71, 71, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(55, 55, 55); }"
                                                                                          "a:visited { color: rgb(55, 55, 55); }" ) ),
                                                                   // read
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(153, 102, 51);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: solid;"
                                                                                          "border-color: rgb(100, 50, 20);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(31, 31, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(31, 31, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(0, 0, 0); }"
                                                                                          "a:visited { color: rgb(0, 0, 0); }" ) ),
                                                                   // disabled
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(153, 102, 51);"
                                                                                          "border-style: none }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(31, 31, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(31, 31, 0);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(215, 208, 0); }"
                                                                                          "a:visited { color: rgb(215, 208, 0); }" ) ),
                                                                   QColor( 153, 102, 51 ) ) );

const ThemeInfo Themes::STYLESHEET_GRAY    = ThemeInfo( QString( "Gray" ),
                                                        // unread
                                                        ThemeData( ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(82, 82, 82);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(60, 60, 60);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(204, 204, 204);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(160, 160, 160); }"
                                                                                          "a:visited { color: rgb(160, 160, 160); }" ) ),
                                                                   // active
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(93, 93, 93);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(113, 113, 113);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(235, 235, 235);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal; }" ),
                                                                                 QString( "a:link { color: rgb(200, 200, 200); }"
                                                                                          "a:visited { color: rgb(200, 200, 200); }" ) ),
                                                                   // read
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(51, 51, 51);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: solid;"
                                                                                          "border-color: rgb(40, 40, 40);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel { background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(203, 203, 203);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(183, 183, 183);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal}" ),
                                                                                 QString( "a:link { color: rgb(140, 140, 140); }"
                                                                                          "a:visited { color: rgb(140, 140, 140); }" ) ),
                                                                   // disabled
                                                                   ThemeElement( QString( "QFrame { background-color: rgb(51, 51, 51);"
                                                                                          "border-style: none }"
                                                                                          "QLabel { background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 153, 153);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 153, 153);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal}" ),
                                                                                 QString( "a:link { color: rgb(215, 208, 100); }"
                                                                                          "a:visited { color: rgb(215, 208, 100); }" ) ),
                                                                   QColor( 51, 51, 51 ) ) );

const ThemeInfo Themes::STYLESHEET_GREEN   = ThemeInfo( QString( "Green" ),
                                                        // unread
                                                        ThemeData( ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(82, 133, 0);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: solid;"
                                                                                          "border-color: rgb(31, 82, 0);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(255, 248, 140); }"
                                                                                          "a:visited { color: rgb(255, 248, 140); }" ) ),
                                                                   // active
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(112, 162, 30);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(82, 133, 0);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(255, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(255, 248, 160); }"
                                                                                          "a:visited { color: rgb(255, 248, 160); }" ) ),
                                                                   // read
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(51, 102, 0);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: solid;"
                                                                                          "border-color: rgb(0, 51, 0);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(224, 224, 224);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(224, 224, 224);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal}" ),
                                                                                 QString( "a:link { color: rgb(204, 186, 78); }"
                                                                                          "a:visited { color: rgb(153, 146, 38); }" ) ),
                                                                   // disabled
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(0, 51, 0);"
                                                                                          "border-style: none }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 153, 153);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 153, 153);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(153, 146, 38); }"
                                                                                          "a:visited { color: rgb(153, 146, 38); }" ) ),
                                                                   QColor( 0, 51, 0 ) ) );

const ThemeInfo Themes::STYLESHEET_PURPLE  = ThemeInfo( QString( "Purple" ),
                                                        // unread
                                                        ThemeData( ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(100, 65, 163);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(90, 50, 150);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(153, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(202, 255, 255); }"
                                                                                          "a:visited { color: rgb(162, 225, 225); }" ) ),
                                                                   // active
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(123, 82, 204);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(103, 63, 184);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(143, 225, 225);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(143, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(202, 255, 255); }"
                                                                                          "a:visited { color: rgb(162, 225, 225); }" ) ),
                                                                   // read
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(82, 31, 133);"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: solid;"
                                                                                          "border-color: rgb(73, 23, 114);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(122, 195, 225);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(102, 175, 205);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(162, 225, 225); }"
                                                                                          "a:visited { color: rgb(162, 225, 225); }" ) ),
                                                                   // disabled
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(102, 51, 153);"
                                                                                          "border-style: none }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(102, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(102, 255, 255);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(153, 146, 38); }"
                                                                                          "a:visited { color: rgb(153, 146, 38); }" ) ),
                                                                   QColor( 102, 51, 153 ) ) );

const ThemeInfo Themes::STYLESHEET_SKY     = ThemeInfo( QString( "Sky" ),
                                                        // unread
                                                        ThemeData( ThemeElement( QString( "QFrame {"
                                                                                          "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
                                                                                          "stop:0 rgba(139, 187, 218, 255),"
                                                                                          "stop:0.5 rgba(180, 209, 236, 255),"
                                                                                          "stop:1 rgba(222, 231, 255, 255));"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(163, 199, 215);"
                                                                                          "border-radius: 12px }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 60, 196);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 60, 196);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(0, 0, 204); }"
                                                                                          "a:visited { color: rgb(0, 0, 204); }" ) ),
                                                                   // active
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
                                                                                          "stop:0 rgba(139, 187, 218, 255),"
                                                                                          "stop:0.2 rgba(180, 209, 236, 255),"
                                                                                          "stop:1 rgba(222, 231, 255, 255));"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(255, 255, 255);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 90, 226);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 90, 226);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(0, 0, 255); }"
                                                                                          "a:visited { color: rgb(0, 0, 255); }" ) ),
                                                                   // read
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1,"
                                                                                          "stop:0 rgba(89, 137, 168, 255),"
                                                                                          "stop:0.8 rgba(180, 209, 236, 255),"
                                                                                          "stop:1 rgba(222, 231, 255, 255));"
                                                                                          "border-width: 2px;"
                                                                                          "border-style: outset;"
                                                                                          "border-color: rgb(102, 137, 153);"
                                                                                          "border-radius: 12px}"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 20, 156);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 20, 156);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(0, 0, 153); }"
                                                                                          "a:visited { color: rgb(0, 0, 153); }" ) ),
                                                                   // disabled
                                                                   ThemeElement( QString( "QFrame {"
                                                                                          "background-color: rgb(180, 209, 236);"
                                                                                          "border-style: none }"
                                                                                          "QLabel {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 20, 156);"
                                                                                          "border-width: 0px;"
                                                                                          "border-radius: 0px }"
                                                                                          "QTextBrowser {"
                                                                                          "background-color: rgba(255, 255, 255, 0);"
                                                                                          "color: rgb(0, 20, 156);"
                                                                                          "border-width: 0px;"
                                                                                          "border-style: normal }" ),
                                                                                 QString( "a:link { color: rgb(0, 0, 153); }"
                                                                                          "a:visited { color: rgb(0, 0, 153); }" ) ),
                                                                   QColor( 184, 202, 215 ) ) );
