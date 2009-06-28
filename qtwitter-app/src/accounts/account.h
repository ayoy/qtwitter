/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QPair>
#include <QMetaType>
#include <twitterapi/twitterapi_global.h>
#include <QDataStream>

struct Account
{
  bool isEnabled;
  TwitterAPI::SocialNetwork network;
  QString login;
  QString password;
  bool directMessages;

  static QPair<TwitterAPI::SocialNetwork,QString> fromString( const QString &name );
  static TwitterAPI::SocialNetwork networkFromString( const QString &name );
  static QString networkToString( TwitterAPI::SocialNetwork network );

  QString toString() const;
  Account operator=( const Account &other );
  bool operator==( const Account &other ) const;
  bool operator<( const Account &other ) const;
};

QDataStream& operator<<( QDataStream &out, const Account &account );
QDataStream& operator>>( QDataStream &in, Account &account );

Q_DECLARE_METATYPE(Account)

#endif // ACCOUNT_H
