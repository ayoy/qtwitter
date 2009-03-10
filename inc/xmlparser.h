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


#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QtXml>
#include "entry.h"

/*!
  \brief A class for parsing XML data from Twitter REST API.

  This class inherits QXmlDefaultHandler and reimplements its methods to comply
  with the structure of XML documents provided by Twitter REST API.
*/
class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

public:

  /*!
    Determines the currently parsed field type.
  */
  enum FieldType {
    None, /*!< Either unsupported or unknown type or parser error. */
    Id, /*!< The status id. */
    Text, /*!< The text (status message). */
    Name, /*!< The owner's screen name.  */
    Login, /*!< The owner's login. */
    Image, /*!< The owner's profile image URL. */
    Homepage, /*!< The owner's homepage. */
    Timestamp /*!< The status timestamp. */
  };

  /*!
    Creates a new XML data parser with a given \a parent.
    \param parent The object's parent.
  */
  XmlParser( QObject *parent = 0 );

  /*!
    Creates a new XML data parser with a given \a parent and the entry type
    to be parsed.
    \param entryType Entry type to be parsed by the parser.
    \param parent The object's parent.
  */
  XmlParser( Entry::Type entryType = Entry::Status, QObject *parent = 0 );

  /*!
    Parser activities at the beginning of XML document.
  */
  virtual bool startDocument();

  /*!
    Parser activities at the end of XML document.
  */
  virtual bool endDocument();

  /*!
    Parser activities at start of the XML element.
  */
  virtual bool startElement( const QString &namespaceURI,
                             const QString &localName,
                             const QString &qName,
                             const QXmlAttributes &atts );

  /*!
    Parser activities at end of the XML element.
  */
  virtual bool endElement( const QString &namespaceURI,
                           const QString &localName,
                           const QString &qName );

  /*!
    Parser activities when reading XML element's text. The actual values are
    being read here.
  */
  virtual bool characters( const QString &ch );

signals:
  /*!
    Emitted when a complete entry is read.
    \param entry A parsed entry.
  */
  void newEntry( Entry *entry );

protected:
  /*!
    Figures out what type of element is currently being parsed.
    \param element The element's text.
    \returns Currently parsed element type.
  */
  FieldType checkFieldType( const QString &element );

  /*!
    Converts a \a timestamp string to a QDateTime object.
    \param timestamp Parsed timestamp string.
    \returns QDateTime-compatible entry timestamp.
  */
  QDateTime toDateTime( const QString &timestamp );

  /*!
    Converts a short month name to a corresponding number.
    \param month A string containing parsed month.
    \returns A month number.
  */
  int getMonth( const QString &month );

  /*!
    Holds the currently processed field type..
  */
  int currentField;

  /*!
    Holds the parsed entry data.
  */
  Entry entry;

  /*!
    Returns true if the currently parsed element is significant for the application.
  */
  bool important;

  static const QByteArray USER_ID; /*!< XML document tag for a status id. */
  static const QByteArray USER_TEXT; /*!< XML document tag for a status text. */
  static const QByteArray USER_NAME; /*!< XML document tag for user's screen name. */
  static const QByteArray USER_LOGIN; /*!< XML document tag for user login. */
  static const QByteArray USER_PHOTO; /*!< XML document tag for user's profile image URL. */
  static const QByteArray USER_HOMEPAGE; /*!< XML document tag for user's homepage. */
  static const QByteArray USER_TIMESTAMP; /*!< XML document tag for status timestamp. */
};

#endif //XMLPARSER_H
