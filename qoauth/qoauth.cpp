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


#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QtDebug>
#include <QEventLoop>
#include <QtCrypto>
#include <QTimer>
#include "qoauth.h"
#include "qoauth_p.h"

const QNetworkRequest::Attribute QOAuthPrivate::requestType = (QNetworkRequest::Attribute) QNetworkRequest::User;

QOAuthPrivate::QOAuthPrivate( QObject *parent ) :
    QObject( parent ),
    manager( new QNetworkAccessManager( this ) ),
    loop( new QEventLoop( this ) ),
    timer( new QTimer( this ) )
{
  connect( manager, SIGNAL(finished(QNetworkReply*)), loop, SLOT(quit()) );
  connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseReply(QNetworkReply*)) );
}

void QOAuthPrivate::parseReply( QNetworkReply *reply )
{
  qDebug() << "reply";
  int returnCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  int operation = reply->request().attribute( QOAuthPrivate::requestType ).toInt();

  qDebug() << returnCode << operation;

  switch ( returnCode ) {
  case 200:

    switch (operation) {
    case QOAuthPrivate::RequestToken:
      qDebug() << reply->readAll();
      break;
    case QOAuthPrivate::Authenticate:
      break;
    case QOAuthPrivate::Authorize:
      break;
    case QOAuthPrivate::AccessToken:
      break;
    default:;
    }

    break;
  case 401:
  case 403:
  default:;
  }
}

QOAuth::QOAuth( QObject *parent ) :
    QObject( parent ),
    d_ptr( new QOAuthPrivate( this ) )
{
  Q_D(QOAuth);

  d->q_ptr = this;
}

QOAuth::~QOAuth()
{
}

void QOAuth::request_token()
{
  Q_D(QOAuth);

  uint time = QDateTime::currentDateTime().toTime_t();

  QCA::Initializer init;

  if( !QCA::isSupported( "hmac(sha1)" ) ) {
    qFatal( "HMAC(SHA1) is not supported!" );
  }

  QCA::InitializationVector iv( 16 );

  QByteArray nonce = iv.toByteArray().toBase64().toPercentEncoding();

  QString timestamp = QString::number( time );
  QByteArray signatureBaseStringFirstPart( "GET&http://twitter.com/oauth/request_token&" );
  QByteArray signatureBaseString = signatureBaseStringFirstPart.toPercentEncoding( "&" );
  QString parametersString( QString ( "oauth_consumer_key=tIFvBZ10xbiOq5p60EcEdA&"
                                      "oauth_nonce=%1&"
                                      "oauth_signature_method=HMAC-SHA1&"
                                      "oauth_timestamp=%2&"
                                      "oauth_version=1.0" ).arg( nonce, timestamp ) );
  QByteArray encodedParametersString = parametersString.toAscii().toPercentEncoding();
  signatureBaseString.append( encodedParametersString );

  QByteArray key( "5gXfCncW8qEilc3kAw05db2bbfw8RNsDhEAEl9iijdA&" );

//  qDebug() << "signature base string:\n" << signatureBaseString;
  QCA::MessageAuthenticationCode hmacObject( "hmac(sha1)", QCA::SymmetricKey( key ) );
  QCA::SecureArray array( signatureBaseString );
  hmacObject.update( array );
  QCA::SecureArray resultArray = hmacObject.final();

  QByteArray digest = resultArray.toByteArray().toBase64();
  QByteArray percentDigest = digest.toPercentEncoding();

  QByteArray authorizationHeader( "oauth_consumer_key=\"tIFvBZ10xbiOq5p60EcEdA\", "
                                  "oauth_signature_method=\"HMAC-SHA1\", "
                                  "oauth_signature=\"" );
  authorizationHeader.append( percentDigest );
  authorizationHeader.append( "\", "
                              "oauth_timestamp=\"" );
  authorizationHeader.append( timestamp );
  authorizationHeader.append( "\", "
                              "oauth_nonce=\"" );
  authorizationHeader.append( nonce );
  authorizationHeader.append( "\", "
                              "oauth_version=\"1.0\"" );

  QNetworkRequest request;
  request.setRawHeader( "Authorization", "OAuth " + authorizationHeader );

  request.setUrl( QUrl( "http://twitter.com/oauth/request_token" ) );
  request.setAttribute( QOAuthPrivate::requestType, QOAuthPrivate::RequestToken );

//  QApplication::clipboard()->setText( QString( "curl -i -H 'Authorization: %1' \"%2\"" ).arg( request.rawHeader( "Authorization" ), request.url().toString() ) );
//  qDebug() << QString( "curl -i -H 'Authorization: %1' \"%2\"" ).arg( request.rawHeader( "Authorization" ), request.url().toString() );
  d->manager->get( request );
//  d->manager->get( QNetworkRequest( QUrl( "http://ayoy.net" ) ) );
  qDebug() << "request" << (int)d->manager;
}

void QOAuth::authorize()
{
}

void QOAuth::authenticate()
{
}

void QOAuth::access_token()
{
}

