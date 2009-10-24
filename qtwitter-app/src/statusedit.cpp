/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#include <QKeyEvent>
#include "statusedit.h"

StatusFilter::StatusFilter( QObject *parent ) : QObject( parent ) {}

bool StatusFilter::eventFilter( QObject *dist, QEvent *event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
        if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
        {
            emit enterPressed();
            return true;
        }
        if ( keyEvent->key() == Qt::Key_Escape )
        {
            emit escPressed();
            return true;
        }
    }
    return QObject::eventFilter(dist, event);
}


const int StatusEdit::STATUS_MAX_LENGTH = 140;

StatusEdit::StatusEdit( QWidget * parent ) :
        QLineEdit( parent ),
        statusClean( true ),
        inReplyToId( 0 )
{
    inactiveColor = palette().color( QPalette::Disabled, QPalette::Text );
    activeColor = palette().color( QPalette::Active, QPalette::Text );
}

void StatusEdit::focusInEvent( QFocusEvent *event )
{
    if ( statusClean ) {
        setText( "" );
        QPalette palette = this->palette();
        palette.setColor( QPalette::Text, activeColor );
        setPalette( palette );
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
    QPalette palette = this->palette();
    palette.setColor( QPalette::Text, inactiveColor );
    setPalette( palette );
    inReplyToId = 0;
    statusClean = true;
}

bool StatusEdit::isStatusClean() const
{
    return statusClean;
}

quint64 StatusEdit::getInReplyTo() const
{
    return inReplyToId;
}

QString StatusEdit::getSelectedUrl() const
{
    return selectedUrl;
}

void StatusEdit::cancelEditing()
{
    initialize();
    clearFocus();
}

void StatusEdit::addReplyString( const QString &name, quint64 inReplyTo )
{
    if ( statusClean ) {
        setText( "@" + name + " ");
        QPalette palette = this->palette();
        palette.setColor( QPalette::Text, activeColor );
        setPalette( palette );
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
    setText( message );
    QPalette palette = this->palette();
    palette.setColor( QPalette::Text, activeColor );
    setPalette( palette );
    statusClean = false;
    setFocus();
    emit textChanged( text() );
}

int StatusEdit::charsLeft() const
{
    return isStatusClean() ? STATUS_MAX_LENGTH : STATUS_MAX_LENGTH - text().length();
}
