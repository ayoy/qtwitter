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


#include <QWidget>
#include <QCursor>
#include <QTimer>
#include "userinfobutton.h"
#include "userinfopopup.h"
#include "userinfo.h"

UserInfoButton::UserInfoButton( QWidget *parent ) :
        QPushButton( parent ),
        userInfo(0)
{
    active = false;
    timer = new QTimer( this );
    timer->setSingleShot( true );
    connect( timer, SIGNAL(timeout()), this, SLOT(showPopup()) );
}

bool UserInfoButton::isPopupActive() const
{
    return (bool)UserInfoPopup::instance();
}

void UserInfoButton::enterEvent( QEvent *event )
{
    Q_UNUSED(event);
    timer->start( 500 );
}

void UserInfoButton::leaveEvent( QEvent *event )
{
    Q_UNUSED(event);
    timer->stop();
}

void UserInfoButton::showPopup()
{
    if ( UserInfoPopup::instance() && UserInfoPopup::instance()->parent() != this )
        destroyPopup();

    if ( !UserInfoPopup::instance() )
        UserInfoPopup::instantiate( status, this, Qt::Popup );

    if ( UserInfoPopup::instance() ) {
        connect( UserInfoPopup::instance(), SIGNAL(closed()), this, SLOT(destroyPopup()) );
        UserInfoPopup::instance()->move( QCursor::pos() + QPoint(-3, -3) );
        UserInfoPopup::instance()->show();
    }
}

void UserInfoButton::destroyPopup()
{
    if ( UserInfoPopup::instance() ) {
        UserInfoPopup::instance()->deleteLater();
        active = false;
    }
}

void UserInfoButton::setData( const Status *status )
{
    this->status = status;
}
