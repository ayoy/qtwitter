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
#include "userinfo.h"
#include <QTimer>

UserInfoPopup* UserInfoPopup::_instance = 0;

UserInfoPopup* UserInfoPopup::instantiate( QWidget *parent, Qt::WindowFlags flags,const Status *status )
{
  if ( !_instance )
    _instance = new UserInfoPopup( parent, flags, status );
  else if ( _instance->parent() != parent ) {
    _instance->deleteLater();
    _instance = new UserInfoPopup( parent, flags, status );
  }

  return _instance;
}

UserInfoPopup* UserInfoPopup::instance()
{
  return _instance;
}

UserInfoPopup::UserInfoPopup( QWidget *parent, Qt::WindowFlags flags, const Status *status ) :
    QWidget( parent, flags ),
    ui( new Ui::UserInfo )
{
  ui->setupUi( this );
  ui->name->setText( status->entry.userInfo.name );
  ui->description->setText( status->entry.userInfo.description );
  ui->friends->setText( QString::number(status->entry.userInfo.friendsCount) );
  ui->location->setText( status->entry.userInfo.location );
  ui->url->setText( status->entry.userInfo.homepage );
  ui->userImage->setPixmap( status->image );
  ui->screenName->setText( status->entry.userInfo.screenName );
}

UserInfoPopup::~UserInfoPopup()
{
  delete ui;
  ui = 0;
  _instance = 0;
}

void UserInfoPopup::close()
{
  emit closed();
  QWidget::close();
}

void UserInfoPopup::leaveEvent( QEvent *event )
{
  Q_UNUSED(event);
  QTimer::singleShot( 500, this, SLOT(close()) );
}

void UserInfoPopup::showEvent( QShowEvent *event )
{
  Q_UNUSED(event);
  QTimer::singleShot( 10000, this, SLOT(close()) );
}
