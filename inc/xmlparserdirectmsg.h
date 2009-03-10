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


#ifndef XMLPARSERDIRECTMSG_H
#define XMLPARSERDIRECTMSG_H

#include "xmlparser.h"

/*!
  \brief A class for parsing XML data of direct messages.

  This class inherits XmlParser and reimplements its methods to comply with
  the structure of XML document for direct messages list provided by Twitter REST API.
*/
class XmlParserDirectMsg : public XmlParser
{
public:

  /*!
    Creates a XML parser for direct messages processing with a given \a parent.
    \param parent An object's parent.
  */
  XmlParserDirectMsg( QObject *parent = 0 );

  /*!
    Parser activities at start of the XML element.
  */
  bool startElement( const QString &namespaceURI,
                     const QString &localName,
                     const QString &qName,
                     const QXmlAttributes &atts );

  /*!
    Parser activities at end of the XML element.
  */
  bool endElement( const QString &namespaceURI,
                   const QString &localName,
                   const QString &qName );

  /*!
    Parser activities when reading XML element's text. The actual values are
    being read here.
  */
  bool characters( const QString &ch );

private:
  bool parsingSender;
};

#endif // XMLPARSERDIRECTMSG_H
