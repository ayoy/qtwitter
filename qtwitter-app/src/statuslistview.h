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


#ifndef STATUSLISTVIEW_H
#define STATUSLISTVIEW_H

#include <QListView>
#include <QKeyEvent>
#include <QDebug>
class StatusListView : public QListView
{
    Q_OBJECT

public:
    StatusListView( QWidget *parent = 0 ) : QListView( parent ) {}

    void keyPressEvent( QKeyEvent *e ) {
        switch ( e->key() ) {
        case Qt::Key_Up:
            emit moveFocus( true );
            e->accept();
            break;
        case Qt::Key_Down:
            emit moveFocus( false );
            e->accept();
            break;
        case Qt::Key_Left:
            emit moveFocusToUnread( true );
            e->accept();
            break;
        case Qt::Key_Right:
            emit moveFocusToUnread( false );
            e->accept();
        default:;
        }
        QListView::keyPressEvent( e );
    }

public slots:
    void clearSelection() {
        emit deselectAll();
        QListView::clearSelection();
    }

signals:
    void moveFocus( bool up );
    void moveFocusToUnread( bool up );
    void deselectAll();

};

#endif // STATUSLISTVIEW_H

/*! \class StatusListView
    \brief A customized QListView class.

    This widget class provides a signal notification when scrolling items using
    keboard's arrow keys. It allows a StatusModel class instance to catch the list's
    key press event and change appropriate Statuses' state accordingly.
*/

/*! \fn StatusListView::StatusListView( QWidget *parent = 0 )
    Creates a new list view with a given \a parent.
*/

/*! \fn void StatusListView::keyPressEvent( QKeyEvent *e )
    This event handler emits moveFocus() when an up or down arrow key was pressed
    or moveFocusToUnread() when an left or right arrow key was pressed.
    \param e A QKeyEvent event's representation.
    \sa moveFocus(), moveFocusToUnread()
*/
