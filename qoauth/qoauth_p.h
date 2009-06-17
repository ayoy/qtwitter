/***************************************************************************
 *   Copyright (C) 2009 by Dominik Kapusta            <d@ayoy.net>         *
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


#ifndef QOAUTH_P_H
#define QOAUTH_P_H

#include <QObject>
#include <QNetworkRequest>

class QNetworkAccessManager;
class QNetworkReply;
class QEventLoop;
class QTimer;
class QOAuth;


class QOAuthPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(QOAuth)

public:
  enum Operation {
    RequestToken,
    Authorize,
    Authenticate,
    AccessToken
  };

  QOAuthPrivate( QObject *parent = 0 );
  QNetworkAccessManager *manager;
  QEventLoop *loop;
  QTimer *timer;

  static const QNetworkRequest::Attribute requestType;

public slots:
  void parseReply( QNetworkReply *reply );

private:
  QOAuth *q_ptr;
};

#endif // QOAUTH_P_H
