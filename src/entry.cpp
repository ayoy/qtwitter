/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "entry.h"

#include <QRegExp>
#include <QDebug>
#include <QList>

Entry::Entry() :
  index( -1 ),
  own( false ),
  userId( -1 ),
  userName( "" ),
  userLogin( "" ),
  userHomepage( "" ),
  hasHomepage( false ),
  userImage( "" ),
  userText( "" )
{
}

Entry::Entry(int itemIndex, int id, const QString &name, const QString &login, const QString &homepage, const QString &image, const QString &text) :
  index( itemIndex ),
  own( false ),
  userId( id ),
  userName( name ),
  userLogin( login ),
  userHomepage( homepage ),
  userImage( image ),
  userText( text )
{
}

Entry::Entry(const Entry &right) :
  index( right.index ),
  own( right.own ),
  userId( right.userId ),
  userName( right.userName ),
  userLogin( right.userLogin ),
  userHomepage( right.userHomepage ),
  hasHomepage( right.hasHomepage ),
  userImage( right.userImage ),
  userText( right.userText )
{
}

bool Entry::checkContents() {
  if ( !hasHomepage ) {
    userHomepage = "";
  }
  if ( userName.compare( "" ) &&
       userLogin.compare( "" ) &&
       userImage.compare( "" ) &&
       userText.compare( "" ) &&
       ( hasHomepage ? userHomepage.compare( "" ) : true ) )
  {
    return true;
  }
  return false;
}

Entry& Entry::operator=( const Entry &right ) {
  index = right.index;
  own = right.own;
  userId = right.userId;
  userName = right.userName;
  userLogin = right.userLogin;
  userHomepage = right.userHomepage;
  hasHomepage = right.hasHomepage;
  userImage = right.userImage;
  userText = right.userText;
  return *this;
}

int Entry::getIndex() const {
  return index;
}

bool Entry::isOwn() const {
  return own;
}

int Entry::id() const {
  return userId;
}

QString Entry::name() const {
  return userName;
}

QString Entry::login() const {
  return userLogin;
}

QString Entry::homepage() const {
  return userHomepage;
}

QString Entry::image() const {
  return userImage;
}

QString Entry::text() const {
  return userText;
}

void Entry::setIndex( int itemIndex ) {
  index = itemIndex;
}

void Entry::setOwn( bool isOwn ) {
  own = isOwn;
}

void Entry::setId( int newId ) {
  userId = newId;
}

void Entry::setName( const QString& newName ) {
  userName = newName;
}

void Entry::setLogin( const QString& newLogin ) {
  userLogin = newLogin;
}

void Entry::setHomepage( const QString& newHomepage ) {
  userHomepage = newHomepage;
}

void Entry::setHasHomepage( bool b ) {
  hasHomepage = b;
}

void Entry::setImage( const QString& newImage ) {
  userImage = newImage;
}

void Entry::setText( const QString& newText ) {
  userText = newText;
  QRegExp ahref( "(http://[^ ]+)( ?)", Qt::CaseInsensitive );
  userText.replace( ahref, "<a href=\\1>\\1</a>\\2" );
  userText.replace( QRegExp( "(^| |[^a-zA-Z0-9])@([^ @.,!:;]+)" ), "\\1<a href=http://twitter.com/\\2>@\\2</a>" );
  ahref.setPattern( "(<a href=[^ ]+)/>" );
  ahref.setMinimal( true );
  userText.replace( ahref, "\\1>" );
}
