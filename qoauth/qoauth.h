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


#ifndef QOAUTH_H
#define QOAUTH_H

#include <QObject>
#include "qoauth_global.h"

class QOAuthPrivate;

class QOAUTH_EXPORT QOAuth : public QObject
{
  Q_OBJECT

public:
  QOAuth( QObject *parent = 0 );
  virtual ~QOAuth();

  void request_token();
  void authorize();
  void authenticate();
  void access_token();

private:
  QOAuthPrivate *d_ptr;
  Q_DECLARE_PRIVATE(QOAuth)
};


#endif // QOAUTH_H
