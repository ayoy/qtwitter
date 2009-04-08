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

class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

public:
  enum FieldType {
    None,
    Id,
    Text,
    Name,
    Login,
    Image,
    Homepage,
    Timestamp
  };

  XmlParser( const QString &login, QObject *parent = 0 );
  XmlParser( const QString &login, Entry::Type entryType = Entry::Status, QObject *parent = 0 );

  virtual bool startDocument();
  virtual bool endDocument();
  virtual bool startElement( const QString &namespaceURI,
                             const QString &localName,
                             const QString &qName,
                             const QXmlAttributes &atts );
  virtual bool endElement( const QString &namespaceURI,
                           const QString &localName,
                           const QString &qName );
  virtual bool characters( const QString &ch );

signals:
  void newEntry( const QString &login, Entry *entry );

protected:
  FieldType checkFieldType( const QString &element );
  QDateTime toDateTime( const QString &timestamp );
  int getMonth( const QString &month );
  QString textToHtml( QString newText );
  QString login;
  int currentField;
  Entry entry;
  bool important;

  static const QByteArray USER_ID;
  static const QByteArray USER_TEXT;
  static const QByteArray USER_NAME;
  static const QByteArray USER_LOGIN;
  static const QByteArray USER_PHOTO;
  static const QByteArray USER_HOMEPAGE;
  static const QByteArray USER_TIMESTAMP;
};

class XmlParserDirectMsg : public XmlParser
{
public:
  XmlParserDirectMsg( const QString &login, QObject *parent = 0 );

  bool startElement( const QString &namespaceURI,
                     const QString &localName,
                     const QString &qName,
                     const QXmlAttributes &atts );
  bool endElement( const QString &namespaceURI,
                   const QString &localName,
                   const QString &qName );
  bool characters( const QString &ch );

private:
  bool parsingSender;
};

#endif //XMLPARSER_H
