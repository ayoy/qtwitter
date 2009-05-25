/***************************************************************************
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


#include <QDebug>
#include "domparser.h"
#include "twitterapi.h"
#include "entry.h"


DomParser::DomParser(TwitterAPI::SocialNetwork network, const QString &login, QObject *parent) :
    QObject( parent ), entry()
{
  this->network = network;
  this->login = login;
}

void DomParser::setContent(const QByteArray &data, int role)
{
  reply.setContent(data, false);
  this->role = role;
}

void DomParser::parse()
{
  switch (role) {
    case TwitterAPI::ROLE_USERINFO:
      parseUserInfo();
      break;
    default:
      qDebug() << "Nothing to parse";
  }
}

void DomParser::parseUserInfo()
{
  QDomElement elem = reply.documentElement();
  QDomNode n = elem.firstChild();
  while ( !n.isNull() ) {
    QDomElement e = n.toElement();
    if ( !e.isNull() ) {
      qDebug() << qPrintable( e.tagName() );
      if(e.tagName() == "id")           //user id
        entry.id = e.text().toInt();
      else if (e.tagName() == "name")  //user real name
        entry.name = e.text();
      else if(e.tagName() == "screen_name")
        entry.screenName = e.text();        //user screen name
      //todo: parse full userinfo; userinfo struct derived from entry
      //todo: if entry.login == 0 then login = text; allows to update existing userinfo, e.g when userinfo was initialized with full entry
    }
    n = n.nextSibling();
  }
//  emit completed(true, url, userId);
//  emit newEntry(network, login, entry);
}

