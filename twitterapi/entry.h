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


#ifndef ENTRY_H
#define ENTRY_H

#include <QString>
#include <QObject>
#include <QDateTime>
#include <QMetaType>
#include "twitterapi.h"

/*!
  \brief A class containing Tweet data.

  This class contains all the parameters for each status or direct message
  extracted by an XML parser.
*/
class TWITTERAPISHARED_EXPORT Entry : public QObject
{
  Q_OBJECT
public:
  /*!
    \brief Type of the entry.

    Needed to specify whether an entry is a status or a direct message.
  */
  enum Type {
    Status, /*!< An entry is a status. */
    DirectMessage /*!< An entry is a direct message. */
  };

  /*! Constructs an empty entry with a given \a entryType and \a parent. */
  Entry( Type entryType = Status, QObject *parent = 0 );

  /*! Constructs an entry as a copy of \a right. */
  Entry( const Entry &right );

  /*! Assigns \a right to this entry and returns a reference to it. */
  Entry& operator=( const Entry &right );

  /*!
    Resets fields of the entry.
    \param resetIndex Indicates if index should be reset.
  */
  void initialize( bool resetIndex = false );

  /*!
    \brief Checks if entry is complete.

    Checks an entry for the existence of required fields and returns true
    if it contains all of the required data.
  */
  bool checkContents();

  Type getType() const; /*!< Returns the type of this entry. */
  int getIndex() const; /*!< Returns the index of this entry. \sa setIndex() */
  bool isOwn() const; /*!< Returns true if entry belongs to user which requests data. \sa setOwn() */
  int id() const; /*!< Returns the id of the entry. \sa setId() */
  QString name() const; /*!< Returns the entry owner's screen name. \sa setName() */
  QString login() const; /*!< Returns the entry owner's login. \sa setLogin() */
  QString homepage() const; /*!< Returns a string with the entry owner's homepage URL. \sa setHomepage() */
  QString image() const; /*!< Returns a string with the entry owner's profile image URL. \sa setImage() */

  /*!
    Returns the text of the entry, processed in order to display URLs and
    references to other users (e.g. \a \@username) as links.
    \sa setText(), originalText()
  */
  QString text() const;
  QString originalText() const; /*!< Returns the raw entry text. */
  QDateTime timestamp() const; /*!< Returns the timestamp of the entry. */

  void setIndex( int itemIndex ); /*!< Sets entry index to \a itemIndex. \sa getIndex() */
  void setOwn( bool isOwn ); /*!< Sets if the entry belongs to a user which requests data. \sa isOwn() */
  void setId( int newId ); /*!< Sets the id of the entry to \a newId. \sa getId() */
  void setName( const QString& newName ); /*!< Sets the entry owner's screen name to \a newName. \sa name() */
  void setLogin( const QString& newLogin ); /*!< Sets the entry owner's login to \a newLogin. \sa login() */
  void setHomepage( const QString& newHomepage ); /*!< Sets the entry owner's homepage URL to \a newHomepage. \sa homepage() */
  void setHasHomepage( bool ); /*!< Sets if the entry owner provides a homepage URL. */
  void setImage( const QString& newImage ); /*!< Sets the entry owner's profile image URL. \sa image() */

  /*!
    Sets the \a originalText to \a newText, and process it in order to display URLs
    and references to other users (e.g. \a \@username) as links.
    \sa text(), originalText()
  */
  void setText( const QString& newText );
  void setTimestamp( const QDateTime& newTimestamp ); /*!< Sets the entry timestamp to \a newTimestamp. \sa timestamp() */

private:
  Type type;
  int index;
  bool own;

  int userId;
  QString userText;
  QString userOriginalText;
  QString userName;
  QString userLogin;
  QString userImage;
  QString userHomepage;
  bool hasHomepage;
  QDateTime userTimestamp;
};

Q_DECLARE_METATYPE(Entry)

#endif //ENTRY_H
