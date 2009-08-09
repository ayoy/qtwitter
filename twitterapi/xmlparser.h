/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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

  XmlParser( const QString &serviceUrl, const QString &login, QObject *parent = 0 );
  XmlParser( const QString &serviceUrl, const QString &login, Entry::Type entryType = Entry::Status, QObject *parent = 0 );

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

  QString textToHtml( QString newText );

signals:
  void newEntry( Entry entry );

protected:
  QDateTime toDateTime( const QString &timestamp );
  int getMonth( const QString &month );
  void parseUserInfo(const QString &ch);
  static inline int getTimeShift();

  QString serviceUrl;
  QString login;

  QString currentTag;
  Entry entry;
  bool important;
  bool parsingUser;
  bool favoritedSet;

  static const int timeShift;
  static const QSet<QString> tags;

  static const QString TAG_STATUS;
  static const QString TAG_USER;
  static const QString TAG_STATUS_ID;
  static const QString TAG_USER_TEXT;
  static const QString TAG_USER_ID;
  static const QString TAG_USER_NAME;
  static const QString TAG_USER_SCREENNAME;
  static const QString TAG_USER_IMAGE;
  static const QString TAG_USER_HOMEPAGE;
  static const QString TAG_USER_TIMESTAMP;
  static const QString TAG_INREPLYTO_STATUS_ID;
  static const QString TAG_INREPLYTO_SCREEN_NAME;
  static const QString TAG_FAVORITED;
  static const QString TAG_LOCATION;
  static const QString TAG_DESCRIPTION;
  static const QString TAG_PROFILE_PROTECTED;
  static const QString TAG_FRIENDS_COUNT;
  static const QString TAG_FOLLOWERS_COUNT;
  static const QString TAG_PROFILE_TIMESTAMP;
  static const QString TAG_UTC_OFFSET;
  static const QString TAG_TIMEZONE;
  static const QString TAG_STATUS_COUNT;
//  static const QString TAG_NOTIFICATIONS;
//  static const QString TAG_FOLLOWING;



private:
  static int calculateTimeShift();

};

class XmlParserDirectMsg : public XmlParser
{
public:
  XmlParserDirectMsg( const QString &serviceUrl, const QString &login, QObject *parent = 0 );

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
