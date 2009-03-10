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

Entry::Entry( Type entryType, QObject *parent ) :
  QObject( parent ),
  type( entryType ),
  index( -1 ),
  own( false ),
  userId( -1 ),
  userText( QString() ),
  userOriginalText( QString() ),
  userName( QString() ),
  userLogin( QString() ),
  userImage( QString() ),
  userHomepage( QString() ),
  hasHomepage( false ),
  userTimestamp( QDateTime() )
{
}

Entry::Entry(const Entry &right) :
  QObject( right.parent() ),
  type( right.type ),
  index( right.index ),
  own( right.own ),
  userId( right.userId ),
  userText( right.userText ),
  userOriginalText( right.userOriginalText ),
  userName( right.userName ),
  userLogin( right.userLogin ),
  userImage( right.userImage ),
  userHomepage( right.userHomepage ),
  hasHomepage( right.hasHomepage ),
  userTimestamp( right.userTimestamp )
{
}

Entry& Entry::operator=( const Entry &right )
{
  type = right.type;
  index = right.index;
  own = right.own;
  userId = right.userId;
  userName = right.userName;
  userLogin = right.userLogin;
  userHomepage = right.userHomepage;
  hasHomepage = right.hasHomepage;
  userImage = right.userImage;
  userText = right.userText;
  userOriginalText = right.userOriginalText;
  userTimestamp = right.userTimestamp;
  return *this;
}

void Entry::initialize( bool resetIndex )
{
  if ( resetIndex ) {
    index = -1;
  }
  own = false;
  userId = -1;
  userName = QString();
  userLogin = QString();
  userHomepage = QString();
  hasHomepage = false;
  userImage = QString();
  userText = QString();
  userOriginalText = QString();
  userTimestamp = QDateTime();
}

bool Entry::checkContents()
{
  if ( !hasHomepage ) {
    userHomepage = QString();
  }
  if ( ( userId != -1 ) &&
       !userName.isNull() &&
       !userLogin.isNull() &&
       ( type == Status ? !userImage.isNull() : true ) &&
       !userText.isNull() &&
       ( hasHomepage ? !userHomepage.isNull() : true ) &&
       !userTimestamp.isNull() ) {
    return true;
  }
  return false;
}

Entry::Type Entry::getType() const { return type; }
int Entry::getIndex() const { return index; }
bool Entry::isOwn() const { return own; }
int Entry::id() const { return userId; }
QString Entry::name() const { return userName; }
QString Entry::login() const { return userLogin; }
QString Entry::homepage() const { return userHomepage; }
QString Entry::image() const { return userImage; }
QString Entry::text() const { return userText; }
QString Entry::originalText() const { return userOriginalText; }
QDateTime Entry::timestamp() const { return userTimestamp; }

void Entry::setIndex( int itemIndex ) { index = itemIndex; }
void Entry::setOwn( bool isOwn ) { own = isOwn; }
void Entry::setId( int newId ) { userId = newId; }
void Entry::setName( const QString& newName ) { userName = newName; }
void Entry::setLogin( const QString& newLogin ) { userLogin = newLogin; }
void Entry::setHomepage( const QString& newHomepage ) { userHomepage = newHomepage; }
void Entry::setHasHomepage( bool b ) { hasHomepage = b; }
void Entry::setImage( const QString& newImage ) { userImage = newImage; }
void Entry::setTimestamp( const QDateTime& newTimestamp ) { userTimestamp = newTimestamp; }

void Entry::setText( const QString& newText )
{
  userOriginalText = newText;
  userText = userOriginalText;
  QRegExp ahref( "(http://[^ ]+)( ?)", Qt::CaseInsensitive );
  userText.replace( ahref, "<a href=\\1>\\1</a>\\2" );
  userText.replace( QRegExp( "(^| |[^a-zA-Z0-9])@([^ @.,!:;]+)" ), "\\1<a href=http://twitter.com/\\2>@\\2</a>" );
  ahref.setPattern( "(<a href=[^ ]+)/>" );
  ahref.setMinimal( true );
  userText.replace( ahref, "\\1>" );
}
