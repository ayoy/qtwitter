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
#include "userinfobutton.h"
#include "userinfopopup.h"

UserInfoButton::UserInfoButton( QWidget *parent ) :
    QPushButton( parent ),
    userInfo(0)
{
  active = false;
}

bool UserInfoButton::isPopupActive() const
{
  return (bool)UserInfoPopup::instance();
}

void UserInfoButton::enterEvent( QEvent *event )
{
  if ( UserInfoPopup::instance() && UserInfoPopup::instance()->parent() != this )
    destroyPopup();

  if ( !UserInfoPopup::instance() )
    UserInfoPopup::instantiate( this, this, Qt::Popup );

  if ( UserInfoPopup::instance() ) {
    connect( UserInfoPopup::instance(), SIGNAL(closed()), this, SLOT(destroyPopup()) );
    UserInfoPopup::instance()->move( QCursor::pos() );
    UserInfoPopup::instance()->show();
    event->accept();
  }
  QPushButton::enterEvent( event );
}

void UserInfoButton::destroyPopup()
{
  if ( UserInfoPopup::instance() ) {
    UserInfoPopup::instance()->deleteLater();
    active = false;
  }
}
