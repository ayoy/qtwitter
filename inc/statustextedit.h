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


#ifndef STATUSTEXTEDIT_H
#define STATUSTEXTEDIT_H

#include <QPlainTextEdit>

class StatusTextEdit : public QPlainTextEdit
{
public:
  StatusTextEdit( QWidget *parent = 0 ) : QPlainTextEdit( parent ) {}

protected:
  void keyPressEvent( QKeyEvent *e )
  {
    if ( e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab ) {
      clearFocus();
      return;
    }
    QPlainTextEdit::keyPressEvent( e );
  }

};

#endif // STATUSTEXTEDIT_H
