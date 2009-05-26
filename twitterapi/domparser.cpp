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


const QString DomParser::TAG_USER_ID = "id";
const QString DomParser::TAG_USER_NAME = "name";
const QString DomParser::TAG_USER_SCREENNAME = "screen_name";
const QString DomParser::TAG_LOCATION = "location";
const QString DomParser::TAG_DESCRIPTION = "description";
const QString DomParser::TAG_USER_IMAGEURL = "profile_image_url";
const QString DomParser::TAG_USER_HOMEPAGE = "url";
const QString DomParser::TAG_PROFILE_PROTECTED = "protected";
const QString DomParser::TAG_FRIENDS_COUNT = "friends_count";
const QString DomParser::TAG_FOLLOWERS_COUNT = "followers_count";
const QString DomParser::TAG_PROFILE_TIMESTAMP = "created_at";
const QString DomParser::TAG_UTC_OFFSET = "utc_offset";
const QString DomParser::TAG_TIMEZONE = "time_zone";
const QString DomParser::TAG_STATUS_COUNT = "statuses_count";
const QString DomParser::TAG_NOTIFICATIONS = "notifications";
const QString DomParser::TAG_FOLLOWING = "following";
const QString DomParser::TAG_USER_STATUSTEXT = "text";


DomParser::DomParser(TwitterAPI::SocialNetwork network, const QString &login, QObject *parent) :
    QObject( parent ), userInfo()
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
      if(e.tagName() == TAG_USER_ID)           //user id
        userInfo.id = e.text().toInt();
      else if (e.tagName() == TAG_USER_NAME)  //user real name
        userInfo.name = e.text();
      else if(e.tagName() == TAG_USER_SCREENNAME)
        userInfo.screenName = e.text();        //user screen name
      else if(e.tagName() == TAG_LOCATION) {
        qDebug() << qPrintable( e.tagName() );
        userInfo.location = e.text();}
      else if(e.tagName() == TAG_DESCRIPTION) {
        qDebug() << qPrintable( e.tagName() );
        userInfo.description == e.text();}
      else if(e.tagName() == TAG_USER_IMAGEURL)
        userInfo.imageUrl = e.text();
      else if(e.tagName() == TAG_USER_HOMEPAGE)
        userInfo.homepage == e.text();
      else if(e.tagName() == TAG_PROFILE_PROTECTED) {
        qDebug() << qPrintable( e.tagName() );
        userInfo.profileProtected = true; }
      else if(e.tagName() == TAG_FRIENDS_COUNT)
        userInfo.friendsCount = e.text().toInt();
      else if(e.tagName() == TAG_FOLLOWERS_COUNT)
        userInfo.followersCount = e.text().toInt();
      else if(e.tagName() == TAG_USER_STATUSTEXT && !userInfo.profileProtected) { //todo: subf
        QDomNode statusTag = e.firstChild();
        while ( !statusTag.isNull() ) {
          QDomElement e = statusTag.toElement();
          if(!e.isNull() ) {
            if(e.tagName() == TAG_USER_STATUSTEXT)
              userInfo.currentStatus = e.text(); //todo: text to html
          }
          statusTag = statusTag.nextSibling();
        }
      }
    }
    n = n.nextSibling();
  }
  emit userInfoCompleted(network, login, userInfo);
}

