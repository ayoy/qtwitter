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


#ifndef TWITPICTEXTEDIT_H
#define TWITPICTEXTEDIT_H

#include <QPlainTextEdit>

class TwitPicTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    TwitPicTextEdit( QWidget *parent = 0 ) :
            QPlainTextEdit( parent ),
            allowEnters( true )
    {}

    void setAllowEnters( bool allow ) { allowEnters = allow; }

signals:
    void enterPressed();

protected:
    void keyPressEvent( QKeyEvent *e )
    {
        if ( e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab ) {
            clearFocus();
            return;
        }
        if ( !allowEnters && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) ) {
            emit enterPressed();
            return;
        }
        QPlainTextEdit::keyPressEvent( e );
    }

private:
    bool allowEnters;

};

#endif // TWITPICTEXTEDIT_H


/*! \class TwitPicTextEdit
    \brief A customized QPlainTextEdit class.

    This class is used by TwitPicView to handle input of the user's message
    added to the uploaded photo. Derives from QPlainTextEdit and reimplements
    keyPressEvent in order to provide focus switching with Tab and Backtab.
*/

/*! \fn TwitPicTextEdit::TwitPicTextEdit( QWidget *parent = 0 )
    Creates a text edit with a given \a parent.
*/

/*! \fn void TwitPicTextEdit::keyPressEvent( QKeyEvent *e )
    Reimplemented from QPlainTextEdit to enable switching focus using Tab
    and Backtab keys.
*/
