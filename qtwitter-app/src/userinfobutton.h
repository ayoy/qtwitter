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


#ifndef USERINFOBUTTON_H
#define USERINFOBUTTON_H

#include <QPushButton>
class Status;

class QWidget;

class UserInfoPopup;

class UserInfoButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( bool popupActive READ isPopupActive )

public:
    UserInfoButton( QWidget *parent = 0 );
    bool isPopupActive() const;
    void setData( const Status *status );

protected:
    void enterEvent( QEvent *event );
    void leaveEvent( QEvent *event );

private slots:
    void destroyPopup();
    void showPopup();

private:
    UserInfoPopup *userInfo;
    QTimer *timer;
    bool active;
    const Status *status;

};

#endif // USERINFOBUTTON_H
