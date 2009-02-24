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


#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QListView>
#include <QKeyEvent>

class StatusList : public QListView
{
  Q_OBJECT

public:
  StatusList( QWidget *parent = 0 ) : QListView( parent ) {}

  void keyPressEvent( QKeyEvent *e ) {
    switch ( e->key() ) {
    case Qt::Key_Up:
      emit moveFocus( true );
      break;
    case Qt::Key_Down:
      emit moveFocus( false );
    default:;
    }
    QListView::keyPressEvent( e );
  }

signals:
  void moveFocus( bool up );

};


#endif // STATUSLIST_H
