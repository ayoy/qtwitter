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


#ifndef STATUSLIST_H
#define STATUSLIST_H

#include <QListView>
#include <QKeyEvent>

/*!
  \brief A customized QListView class.

  This widget class provides a signal notification when scrolling items using
  keboard's arrow keys. It allows a TweetModel class instance to catch the list's
  key press event and change appropriate Tweets' state accordingly.
*/
class StatusList : public QListView
{
  Q_OBJECT

public:
  /*!
    Creates a new list view with a given \a parent.
  */
  StatusList( QWidget *parent = 0 ) : QListView( parent ) {}

  /*!
    This event handler emits moveFocus() when an up or down arrow key was pressed.
    \param e A QKeyEvent event's representation.
    \sa moveFocus()
  */
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
  /*!
    Emitted when an up or down arrow key was pressed.
    \param up True when up arrow key was pressed, false when it was down arrow key.
    \sa keyPressEvent()
  */
  void moveFocus( bool up );

};

#endif // STATUSLIST_H
