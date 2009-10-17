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


#ifndef USERINFOPOPUP_H
#define USERINFOPOPUP_H

#include <QWidget>
#include "ui_userinfopopup.h"
#include "statuslist.h"

class QEvent;
class QShowEvent;

class UserInfoPopup : public QWidget
{
    Q_OBJECT

public:
    virtual ~UserInfoPopup();

    static UserInfoPopup* instantiate( const Status *status, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    static UserInfoPopup* instance();

public slots:
    void close();

signals:
    void closed();

protected:
    UserInfoPopup( const Status *status, QWidget *parent = 0, Qt::WindowFlags flags = 0 );

    void leaveEvent( QEvent *event );
    void showEvent( QShowEvent *event );

private:
    static UserInfoPopup *_instance;
    Ui::UserInfoPopup *ui;
};

#endif // USERINFOPOPUP_H
