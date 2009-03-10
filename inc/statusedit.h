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


#ifndef STATUSEDIT_H
#define STATUSEDIT_H

#include <QLineEdit>

/*!
  \brief A customized QLineEdit class.

  This class inherits from QLineEdit and reimplements focus events so as to fit
  to the application requirements.
*/
class StatusEdit : public QLineEdit
{
  Q_OBJECT
public:

  static const int STATUS_MAX_LENGTH; /*!< This value stores maximum length of the posted update. */

  /*!
    Creates a new status edit field object, with a given \a parent.
  */
  StatusEdit( QWidget * parent = 0 );

  /*!
    Clears the status edit field if it contained "What are you doing?" when entering.
    \param event A QFocusEvent event's representation.
    \sa focusOutEvent(), initialize()
  */
  void focusInEvent( QFocusEvent * event );

  /*!
    Makes the status edit field initialized if it contains an empty
    string when losing focus.
    \param event A QFocusEvent event's representation.
    \sa focusInEvent(), initialize()
  */
  void focusOutEvent( QFocusEvent * event );

  /*!
    Clears status and sets the status edit field to initial "What are you doing?".
  */
  void initialize();

  /*!
    Checks wether the status edit field is initialized.
    \returns true when the status edit field is initialized, otherwise returns false.
  */
  bool isStatusClean() const;

  /*!
    Gets the reply status Id if exists.
    \returns Id of the status to which a reply is posted. If the status is not a reply, returns -1.
  */
  int getInReplyTo() const;

public slots:
  /*!
    Initializes status edit field and clears focus.
  */
  void cancelEditing();

  /*!
    Adds user login in Twitter replying convention (\a \@user).
    \param name User login to be added to status edit field.
    \param inReplyTo Id of the existing status to which the reply is posted.
  */
  void addReplyString( const QString &name, int inReplyTo );

  /*!
    Adds a retweet message to status field.
    \param message Message to be added to status edit field.
  */
  void addRetweetString( QString message );

signals:
  /*!
    Emitted to inform user about encountered problems.
    \param message Error message.
  */
  void errorMessage( const QString &message );

private:
  bool statusClean;
  int inReplyToId;
};

#endif //STATUSEDIT_H
