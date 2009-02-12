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
  id( -1 ),
  userName( "" ),
  userLogin( "" ),
  userImage( "" ),
  userText( "" )
{
}

Entry::Entry(int itemId, const QString &name, const QString &login, const QString &image, const QString &text) :
  id ( itemId ),
  userName( name ),
  userLogin( login ),
  userImage( image ),
  userText( text )
{
}

Entry::Entry(const Entry &right) :
  userName( right.userName ),
  userLogin( right.userLogin ),
  userImage( right.userImage ),
  userText( right.userText )
{
}

bool Entry::checkContents() {
  if ( userName.compare( "" ) &&
       userLogin.compare( "" ) &&
       userImage.compare( "" ) &&
       userText.compare( "" ) ) {
    return true;
  }
  return false;
}

Entry& Entry::operator=( const Entry &right ) {
  userName = right.userName;
  userLogin = right.userLogin;
  userImage = right.userImage;
  userText = right.userText;
  return *this;
}

int Entry::getId() const {
  return id;
}

QString Entry::name() const {
  return userName;
}

QString Entry::login() const {
  return userLogin;
}

QString Entry::image() const {
  return userImage;
}

QString Entry::text() const {
  return userText;
}

void Entry::setId( int itemId ) {
  id = itemId;
}

void Entry::setName( const QString& newName ) {
  userName = newName;
}

void Entry::setLogin( const QString& newLogin ) {
  userLogin = newLogin;
}

void Entry::setImage( const QString& newImage ) {
  userImage = newImage;
}

void Entry::setText( const QString& newText ) {
  userText = newText;
  QRegExp ahref( "(http://[^ ]+)( ?)", Qt::CaseInsensitive );
  userText.replace( ahref, "<a href=\\1>\\1</a>\\2" );
  userText.replace( QRegExp( "(^| )@([^ ]+)( ?)" ), " <a href=http://twitter.com/\\2>@\\2</a>\\3" );
  ahref.setPattern( "(<a href=[^ ]+)/>" );
  ahref.setMinimal( true );
  userText.replace( ahref, "\\1>" );
}
