/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#include "statusedit.h"

const int StatusEdit::STATUS_MAX_LENGTH = 140;

StatusEdit::StatusEdit( QWidget * parent ) :
  QLineEdit( parent ),
  statusClean( true ),
  inReplyToId( -1 )
  {}

void StatusEdit::focusInEvent( QFocusEvent *event )
{
  if ( statusClean ) {
    setText( "" );
    statusClean = false;
  }
  QLineEdit::focusInEvent( event );
}

void StatusEdit::focusOutEvent( QFocusEvent *event )
{
  if ( !text().compare( "" ) ) {
    initialize();
  }
  QLineEdit::focusOutEvent( event );
}

void StatusEdit::initialize()
{
  setText( tr( "What are you doing?" ) );
  inReplyToId = -1;
  statusClean = true;
}

bool StatusEdit::isStatusClean() const
{
  return statusClean;
}

int StatusEdit::getInReplyTo() const
{
  return inReplyToId;
}

void StatusEdit::cancelEditing()
{
  initialize();
  clearFocus();
}

void StatusEdit::addReplyString( const QString &name, int inReplyTo )
{
  if ( statusClean ) {
    setText( "@" + name + " ");
    statusClean = false;
  } else {
    insert( "@" + name + " ");
  }
  inReplyToId = inReplyTo;
  setFocus();
  emit textChanged( text() );
  update();
}

void StatusEdit::addRetweetString( QString message )
{
  if ( message.length() > StatusEdit::STATUS_MAX_LENGTH ) {
    emit errorMessage( tr( "The message is too long and it will be truncated." ) );
    message.truncate( StatusEdit::STATUS_MAX_LENGTH );
  }
  setText( message );
  statusClean = false;
  setFocus();
  emit textChanged( text() );
}
