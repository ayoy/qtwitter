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

#include "qoauth.h"
#include <QObject>
#include <QNetworkRequest>
#include <QMap>

class QNetworkAccessManager;
class QNetworkReply;
class QEventLoop;
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

  enum ParsingMode {
    ParseForSignatureBaseString,
    ParseForInlineQuery = ParseForSignatureBaseString,
    ParseForHeaderArguments
  };

  QOAuthPrivate( QObject *parent = 0 );
  QByteArray httpMethodToString( QOAuth::HttpMethod method );
  QByteArray signatureMethodToString( QOAuth::SignatureMethod method );

  QByteArray createParametersString( const QOAuth::ParamMap &parameters, ParsingMode mode );
  QByteArray createSignature( const QString &requestUrl, QOAuth::SignatureMethod signatureMethod,
                              QOAuth::HttpMethod httpMethod, QOAuth::ParamMap *params );

  bool parseRequestTokenReply( const QByteArray &data );
  bool parseAccessTokenReply( const QByteArray &data );

  QOAuth::ParamMap replyToMap( const QByteArray &data );

  QByteArray consumerKey;
  QByteArray consumerSecret;
  QByteArray token;
  QByteArray tokenSecret;
  QByteArray accessToken;

  QOAuth::ParamMap replyParams;

  QNetworkAccessManager *manager;
  QEventLoop *loop;

  static const QNetworkRequest::Attribute RequestType;

public slots:
  void parseReply( QNetworkReply *reply );

protected:
  QOAuth *q_ptr;
};

#endif // QOAUTH_P_H
