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


#ifndef STATUSFILTER_H
#define STATUSFILTER_H

#include <QObject>
#include <QEvent>
#include <qevent.h>

class StatusFilter : public QObject
{
  Q_OBJECT

public:
  StatusFilter( QObject *parent = 0 ) : QObject( parent ) {}

signals:
  void enterPressed();
  void escPressed();

protected:
  bool eventFilter( QObject *dist, QEvent *event )
  {
    if ( event->type() == QEvent::KeyPress )
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
      if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
      {
        emit enterPressed();
        return true;
      }
      if ( keyEvent->key() == Qt::Key_Escape )
      {
        emit escPressed();
        return true;
      }
    }
    return QObject::eventFilter(dist, event);
  }
};

#endif //STATUSFILTER_H
