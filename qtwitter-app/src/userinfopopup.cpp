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


#include "userinfopopup.h"

UserInfoPopup* UserInfoPopup::_instance = 0;

UserInfoPopup* UserInfoPopup::instantiate( QObject *origin, QWidget *parent, Qt::WindowFlags flags )
{
  if ( !_instance )
    _instance = new UserInfoPopup( origin, parent, flags );
  else if ( _instance->_origin != origin ) {
    _instance->deleteLater();
    _instance = new UserInfoPopup( origin, parent, flags );
  }

  return _instance;
}

UserInfoPopup* UserInfoPopup::instance()
{
  return _instance;
}

QObject * const UserInfoPopup::origin() const
{
  return _origin;
}

UserInfoPopup::UserInfoPopup( QObject *origin, QWidget *parent, Qt::WindowFlags flags ) :
    QWidget( parent, flags ),
    _origin( origin ),
    ui( new Ui::UserInfo )
{
  ui->setupUi( this );
}

UserInfoPopup::~UserInfoPopup()
{
  delete ui;
  ui = 0;
  _instance = 0;
}

void UserInfoPopup::leaveEvent( QEvent *event )
{
  Q_UNUSED(event);
  close();
  emit closed();
}
