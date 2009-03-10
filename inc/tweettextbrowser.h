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


#ifndef TWEETTEXTBROWSER_H
#define TWEETTEXTBROWSER_H

#include <QTextBrowser>
#include <QKeyEvent>

/*!
  \brief A customized QTextBrowser class.

  This class inherits QTextBrowser and provides signalling for mouse press events
  and ignoring key press events.
*/
class TweetTextBrowser : public QTextBrowser
{
  Q_OBJECT
public:

  /*!
    Creates a new text browser widget with a given \a parent.
    \param parent The widget's parent.
  */
  TweetTextBrowser( QWidget *parent = 0 ) : QTextBrowser( parent ) {}

  /*!
    Customized to emit \ref mousePressed() on every mouse press event.
    \param e A QMouseEvent event's representation.
    \sa mousePressed()
  */
  void mousePressEvent ( QMouseEvent * e )
  {
    emit mousePressed();
    QTextBrowser::mousePressEvent( e );
  }

  /*!
    Customized to ignore all the key press events, so that they could be
    passed on to the parent widget.
    \param e A QKeyEvent event's representation.
  */
  void keyPressEvent( QKeyEvent *e )
  {
    e->ignore();
  }

signals:
  /*!
    Emitted when a widget receives a mouse press event.
    \sa mousePressEvent()
  */
  void mousePressed();

};

#endif // TWEETTEXTBROWSER_H
