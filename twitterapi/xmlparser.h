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


#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QXmlDefaultHandler>
#include <QSet>
#include "twitterapi.h"
#include "entry.h"

class XmlParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT

public:

  XmlParser( TwitterAPI::SocialNetwork network, const QString &login, QObject *parent = 0 );
  XmlParser( TwitterAPI::SocialNetwork network, const QString &login, Entry::Type entryType = Entry::Status, QObject *parent = 0 );

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
  void newEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry );

protected:
  QDateTime toDateTime( const QString &timestamp );
  int getMonth( const QString &month );
  QString textToHtml( QString newText );
  static inline int getTimeShift();

  TwitterAPI::SocialNetwork network;
  QString login;

  QString currentTag;
  Entry entry;
  bool important;

  static const int timeShift;
  static const QSet<QString> tags;

  static const QString TAG_STATUS;
  static const QString TAG_USER_ID;
  static const QString TAG_USER_TEXT;
  static const QString TAG_USER_NAME;
  static const QString TAG_USER_LOGIN;
  static const QString TAG_USER_IMAGE;
  static const QString TAG_USER_HOMEPAGE;
  static const QString TAG_USER_TIMESTAMP;

private:
//  static void populateTagsSet();
  static int calculateTimeShift();

};

class XmlParserDirectMsg : public XmlParser
{
public:
  XmlParserDirectMsg( TwitterAPI::SocialNetwork network, const QString &login, QObject *parent = 0 );

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

  static const QString TAG_DIRECT_MESSAGE;
  static const QString TAG_SENDER;
};

#endif //XMLPARSER_H
