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

#define USER_STATUS "text"
#define USER_LOGIN "name"
#define USER_PHOTO "profile_image_url"

class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

public:
  enum FieldType {
    None,
    Name,
    Image,
    Text
  };

  enum XmlType {
    One,
    All
  };

private:
  int lastField;
  XmlType type;
  int sendIn;
  Entry entry;
  bool important;
  int checkFieldType( const QString &element );
  
public:
  XmlParser();
  XmlParser( XmlType type );
  
  bool startDocument();
  bool endDocument();
  
  bool startElement( const QString &namespaceURI,
                     const QString &localName,
                     const QString &qName,
                     const QXmlAttributes &atts );
  bool endElement( const QString &namespaceURI,
                   const QString &localName,
                   const QString &qName );
                   
  bool characters( const QString &ch );

signals:
  void dataParsed( const QString &text );
  void newEntry( const Entry &entry, XmlParser::XmlType type );
  void xmlParsed();

};

#endif //XMLPARSER_H
