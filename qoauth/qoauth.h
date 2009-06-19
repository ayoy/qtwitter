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
#include <QMap>
#include "qoauth_global.h"

class QOAuthPrivate;

class QOAUTH_EXPORT QOAuth : public QObject
{
  Q_OBJECT

  Q_PROPERTY( QByteArray consumerKey READ consumerKey WRITE setConsumerKey )
  Q_PROPERTY( QByteArray consumerSecret READ consumerSecret WRITE setConsumerSecret )
  Q_PROPERTY( QByteArray token READ token )

public:
  enum SignatureMethod {
    HMAC_SHA1,
    HMAC_RSA,
    PLAINTEXT
  };

  enum HttpMethod {
    GET,
    POST
  };
  
  typedef QMap<QByteArray,QByteArray> MiscParams;

  static const QByteArray Version;

  QOAuth( QObject *parent = 0 );
  virtual ~QOAuth();

  QByteArray consumerKey() const;
  void setConsumerKey( const QByteArray &consumerKey );

  QByteArray consumerSecret() const;
  void setConsumerSecret( const QByteArray &consumerSecret );

  QByteArray token() const;

  QByteArray requestToken( const QString &requestUrl, SignatureMethod signatureMethod, HttpMethod httpMethod,
                           uint timeout = 0, const MiscParams &params = MiscParams() );
//  void authorize();
  void authenticate();
  QByteArray accessToken( const QString &requestUrl, SignatureMethod signatureMethod, HttpMethod httpMethod,
                    uint timeout = 0, const MiscParams &params = MiscParams() );


protected:
  QOAuthPrivate *d_ptr;

private:
  Q_DECLARE_PRIVATE(QOAuth)
};


#endif // QOAUTH_H
