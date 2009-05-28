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


#ifndef DOMPARSER_H
#define DOMPARSER_H

#include <QObject>
#include <QDomDocument>
#include "userinfo.h"
#include "twitterapi.h"

class DomParser : public QObject
{
  Q_OBJECT

public:
  DomParser(TwitterAPI::SocialNetwork network, const QString &login, QObject *parent);
  void parse();
  void setContent(const QByteArray &data, int role);

signals:
  void userInfoCompleted( TwitterAPI::SocialNetwork network, const QString &login, UserInfo userinfo );

protected:
  void parseUserInfo();
  void parseStatusMsg();

  static const QString TAG_STATUS;
  static const QString TAG_USER_ID;
  static const QString TAG_USER_NAME;
  static const QString TAG_USER_SCREENNAME;
  static const QString TAG_LOCATION;
  static const QString TAG_DESCRIPTION;
  static const QString TAG_USER_IMAGEURL;
  static const QString TAG_USER_HOMEPAGE;
  static const QString TAG_PROFILE_PROTECTED;
  static const QString TAG_FRIENDS_COUNT;
  static const QString TAG_FOLLOWERS_COUNT;
  static const QString TAG_PROFILE_TIMESTAMP;
  static const QString TAG_UTC_OFFSET;
  static const QString TAG_TIMEZONE;
  static const QString TAG_STATUS_COUNT;
  static const QString TAG_NOTIFICATIONS;
  static const QString TAG_FOLLOWING;
  static const QString TAG_USER_STATUSTEXT;


private:
  QDomDocument reply;
  TwitterAPI::SocialNetwork network;
  QString login;
  UserInfo userInfo;
  int role;
};

#endif // DOMPARSER_H
