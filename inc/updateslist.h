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


#ifndef UPDATESLIST_H
#define UPDATESLIST_H

#include <QListView>

#include <QMouseEvent>
#include <QDebug>

class UpdatesList : public QListView {

  Q_OBJECT
public:
  UpdatesList( QWidget *parent ) : QListView( parent ) {}

signals:
  void contextMenuRequested();

private:
  void mousePressEvent( QMouseEvent *event ) {
    if ( event->button() == Qt::RightButton ) {
      qDebug() << "right click on the list";
      emit contextMenuRequested();
    }
    QListView::mousePressEvent( event );
  }
};

#endif // UPDATESLIST_H
