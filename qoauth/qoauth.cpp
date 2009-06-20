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


#include "qoauth.h"
#include "qoauth_p.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QtDebug>
#include <QEventLoop>
#include <QtCrypto>
#include <QTimer>
#include <QMap>

const QByteArray QOAuth::Version = "1.0";

const QNetworkRequest::Attribute QOAuthPrivate::RequestType = (QNetworkRequest::Attribute) QNetworkRequest::User;

const QByteArray QOAuth::ParamConsumerKey     = "oauth_consumer_key";
const QByteArray QOAuth::ParamNonce           = "oauth_nonce";
const QByteArray QOAuth::ParamSignature       = "oauth_signature";
const QByteArray QOAuth::ParamSignatureMethod = "oauth_signature_method";
const QByteArray QOAuth::ParamTimestamp       = "oauth_timestamp";
const QByteArray QOAuth::ParamVersion         = "oauth_version";
const QByteArray QOAuth::ParamToken           = "oauth_token";
const QByteArray QOAuth::ParamTokenSecret     = "oauth_token_secret";
const QByteArray QOAuth::ParamAccessToken     = "oauth_access_token";

QOAuthPrivate::QOAuthPrivate( QObject *parent ) :
    QObject( parent ),
    consumerKey( QByteArray() ),
    consumerSecret( QByteArray() ),
    accessToken( QByteArray() ),
    manager( new QNetworkAccessManager( this ) ),
    loop( new QEventLoop( this ) )
{
  connect( manager, SIGNAL(finished(QNetworkReply*)), loop, SLOT(quit()) );
  connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseReply(QNetworkReply*)) );
}

QByteArray QOAuthPrivate::httpMethodToString( QOAuth::HttpMethod method )
{
  switch ( method ) {
  case QOAuth::GET:
    return "GET";
  case QOAuth::POST:
    return "POST";
  default:
    qWarning() << __PRETTY_FUNCTION__ << "Unrecognized method";
    return QByteArray();
  }
}

QByteArray QOAuthPrivate::signatureMethodToString( QOAuth::SignatureMethod method )
{
  switch ( method ) {
  case QOAuth::HMAC_SHA1:
    return "HMAC-SHA1";
  case QOAuth::HMAC_RSA:
    return "HMAC-RSA";
  case QOAuth::PLAINTEXT:
    return "PLAINTEXT";
  default:
    qWarning() << __PRETTY_FUNCTION__ << "Unrecognized method";
    return QByteArray();
  }
}

QByteArray QOAuthPrivate::createParametersString( const QOAuth::ParamMap &parameters, QOAuth::ParsingMode mode )
{
  QByteArray middleString;
  QByteArray endString;
  switch ( mode ) {
  // equals to QOAuth::ParseForInlineQuery
  case QOAuth::ParseForSignatureBaseString:
    middleString = "=";
    endString = "&";
    break;
  case QOAuth::ParseForHeaderArguments:
    middleString = "=\"";
    endString = "\",";
    break;
  default:
    qWarning() << __PRETTY_FUNCTION__ << "Unrecognized mode";
    return QByteArray();
  }

  QByteArray parameter;
  QByteArray parametersString;

  foreach( parameter, parameters.uniqueKeys() ) {
    QList<QByteArray> values = parameters.values( parameter );
    if ( values.size() > 1 ) {
      qSort( values.begin(), values.end() );
    }
    QByteArray value;
    foreach ( value, values ) {
      parametersString.append( parameter );
      parametersString.append( middleString );
      parametersString.append( value );
      parametersString.append( endString );
    }
  }
  parametersString.chop(1);

  return parametersString;
}

void QOAuthPrivate::parseReply( QNetworkReply *reply )
{
  qDebug() << "reply";
  int returnCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  int operation = reply->request().attribute( QOAuthPrivate::RequestType ).toInt();

  qDebug() << returnCode << operation;

  switch ( returnCode ) {
  case 200:

    switch (operation) {
    case QOAuthPrivate::RequestToken:
      parseRequestTokenReply( reply->readAll() );
      break;
    case QOAuthPrivate::Authenticate:
      break;
    case QOAuthPrivate::Authorize:
      break;
    case QOAuthPrivate::AccessToken:
      parseAccessTokenReply( reply->readAll() );
      break;
    default:;
    }

    break;
  case 401:
  case 403:

  default:;
  }
}

QOAuth::ParamMap QOAuthPrivate::replyToMap( const QByteArray &data )
{
  // split reply to name=value strings
  QList<QByteArray> replyParams = data.split( '&' );
//  // we'll store them in a map
  QOAuth::ParamMap parameters;

  QByteArray replyParam;
  QByteArray key;
  int separatorIndex;

  // iterate through name=value pairs
  foreach ( replyParam, replyParams ) {
    // find occurrence of '='
    separatorIndex = replyParam.indexOf( '=' );
    // key is on the left
    key = replyParam.left( separatorIndex );
    // value is on the right
    parameters.insert( key , replyParam.right( replyParam.length() - separatorIndex - 1 ) );
  }

  return parameters;
}

bool QOAuthPrivate::parseRequestTokenReply( const QByteArray &data )
{
  replyParams = replyToMap( data );

  if ( !replyParams.contains( QOAuth::ParamToken ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token not present in reply!";
    return false;
  }
  if ( !replyParams.contains( QOAuth::ParamTokenSecret ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token_secret not present in reply!";
    return false;
  }  
  return true;
}

bool QOAuthPrivate::parseAccessTokenReply( const QByteArray &data )
{
  replyParams = replyToMap( data );

  if ( !replyParams.contains( QOAuth::ParamToken ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token not present in reply!";
    return false;
  }
  if ( !replyParams.contains( QOAuth::ParamTokenSecret ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token_secret not present in reply!";
    return false;
  }
  return true;
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

QByteArray QOAuth::consumerKey() const
{
  Q_D(const QOAuth);

  return d->consumerKey;
}

void QOAuth::setConsumerKey( const QByteArray &consumerKey )
{
  Q_D(QOAuth);

  d->consumerKey = consumerKey;
}

QByteArray QOAuth::consumerSecret() const
{
  Q_D(const QOAuth);

  return d->consumerSecret;
}

void QOAuth::setConsumerSecret( const QByteArray &consumerSecret )
{
  Q_D(QOAuth);

  d->consumerSecret = consumerSecret;
}

QByteArray QOAuthPrivate::createSignature( const QString &requestUrl, QOAuth::HttpMethod httpMethod,
                                           QOAuth::SignatureMethod signatureMethod, const QByteArray &token,
                                           const QByteArray &tokenSecret, QOAuth::ParamMap *params )
{
  QCA::Initializer init;

  if( !QCA::isSupported( "hmac(sha1)" ) ) {
    qFatal( "HMAC(SHA1) is not supported!" );
  }

  // create nonce
  QCA::InitializationVector iv( 16 );
  QByteArray nonce = iv.toByteArray().toBase64().toPercentEncoding();

  // create timestamp
  uint time = QDateTime::currentDateTime().toTime_t();
  QByteArray timestamp = QByteArray::number( time );

  // create signature base string
  // 1. create the method string
  QByteArray httpMethodString = httpMethodToString( httpMethod );
  // 2. prepare percent-encoded request URL
  QByteArray percentRequestUrl = requestUrl.toAscii().toPercentEncoding();
  // 3. prepare percent-encoded parameters string
  params->insert( QOAuth::ParamConsumerKey, consumerKey );
  params->insert( QOAuth::ParamNonce, nonce );
  params->insert( QOAuth::ParamSignatureMethod,
                  signatureMethodToString( signatureMethod ) );
  params->insert( QOAuth::ParamTimestamp, timestamp );
  params->insert( QOAuth::ParamVersion, QOAuth::Version );
  if ( !token.isEmpty() ) {
    params->insert( QOAuth::ParamToken, token );
  }

  QByteArray parametersString = createParametersString( *params, QOAuth::ParseForSignatureBaseString );
  QByteArray percentParametersString = parametersString.toPercentEncoding();

  // 4. create signature base string
  QByteArray signatureBaseString;
  signatureBaseString.append( httpMethodString + "&" );
  signatureBaseString.append( percentRequestUrl + "&" );
  signatureBaseString.append( percentParametersString );

  // create key for HMAC-SHA1 hashing
  QByteArray key( consumerSecret + "&" + tokenSecret );

  // create HMAC-SHA1 digest in Base64
  QCA::MessageAuthenticationCode hmac( "hmac(sha1)", QCA::SymmetricKey( key ) );
  QCA::SecureArray array( signatureBaseString );
  hmac.update( array );
  QCA::SecureArray resultArray = hmac.final();
  QByteArray digest = resultArray.toByteArray().toBase64();
  // percent-encode the digest
  QByteArray signature = digest.toPercentEncoding();
  return signature;
}


QOAuth::ParamMap QOAuth::requestToken( const QString &requestUrl, HttpMethod httpMethod, SignatureMethod signatureMethod,
                                       uint timeout, const ParamMap &params )
{
  Q_D(QOAuth);

  if ( d->consumerKey.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer key is empty, make sure that you set it with QOAuth::setConsumerKey()";
    return ParamMap();
  }
  if ( d->consumerSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer secret is empty, make sure that you set it with QOAuth::setConsumerSecret()";
    return ParamMap();
  }

  // temporarily only HMAC-SHA1 is supported
  if ( signatureMethod != HMAC_SHA1 ) {
    qWarning() << __PRETTY_FUNCTION__ << "Sorry, we're currently supporting only HMAC-SHA1 method...";
    return ParamMap();
  }

  QOAuth::ParamMap parameters = params;
  // create signature
  QByteArray signature = d->createSignature( requestUrl, httpMethod, signatureMethod,
                                             QByteArray(), QByteArray(), &parameters );

  // add signature to parameters and create authorization header
  parameters.insert( QOAuth::ParamSignature, signature );
  QByteArray authorizationHeader = d->createParametersString( parameters, ParseForHeaderArguments );

  // create a network request
  QNetworkRequest request;
  request.setRawHeader( "Authorization", "OAuth " + authorizationHeader );
  request.setUrl( QUrl( requestUrl ) );
  request.setAttribute( QOAuthPrivate::RequestType, QOAuthPrivate::RequestToken );

//  qDebug() << QString( "curl -i -H 'Authorization: %1' \"%2\"" ).arg( request.rawHeader( "Authorization" ), request.url().toString() );
  // fire up a single shot timer if timeout was specified
  if ( timeout > 0 ) {
    QTimer::singleShot( timeout, d->loop, SLOT(quit()) );
  }
  // send the request
  d->manager->get( request );
  // start the event loop and wait for the response
  d->loop->exec();

  return d->replyParams;
}

QOAuth::ParamMap QOAuth::accessToken( const QString &requestUrl, HttpMethod httpMethod, SignatureMethod signatureMethod,
                                      const QByteArray &token, const QByteArray &tokenSecret,
                                      uint timeout, const ParamMap &params )
{
  Q_D(QOAuth);

  if ( d->consumerKey.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer key is empty, make sure that you set it with QOAuth::setConsumerKey()";
    return ParamMap();
  }
  if ( d->consumerSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer secret is empty, make sure that you set it with QOAuth::setConsumerSecret()";
    return ParamMap();
  }

  // temporarily only HMAC-SHA1 is supported
  if ( signatureMethod != HMAC_SHA1 ) {
    qWarning() << __PRETTY_FUNCTION__ << "Sorry, we're currently supporting only HMAC-SHA1 method...";
    return ParamMap();
  }

  QCA::Initializer init;

  if( !QCA::isSupported( "hmac(sha1)" ) ) {
    qFatal( "HMAC(SHA1) is not supported!" );
  }

  QOAuth::ParamMap parameters = params;
  QByteArray signature = d->createSignature( requestUrl, httpMethod, signatureMethod,
                                             token, tokenSecret, &parameters );

  // add signature to parameters and create authorization header
  parameters.insert( QOAuth::ParamSignature, signature );
  QByteArray authorizationHeader = d->createParametersString( parameters, ParseForInlineQuery );

  // create a network request
  QNetworkRequest request;
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  request.setUrl( QUrl( requestUrl ) );
  request.setAttribute( QOAuthPrivate::RequestType, QOAuthPrivate::AccessToken );

//  qDebug() << QString( "curl -i -d '%1' %2" ).arg( QString(authorizationHeader) ).arg( request.url().toString() );
  // fire up a single shot timer if timeout was specified
  if ( timeout > 0 ) {
    QTimer::singleShot( timeout, d->loop, SLOT(quit()) );
  }
  // send the request
  d->manager->post( request, authorizationHeader );
  // start the event loop and wait for the response
  d->loop->exec();

  return d->replyParams;
}

QByteArray QOAuth::createParametersString( const QString &requestUrl, QOAuth::HttpMethod httpMethod, QOAuth::SignatureMethod signatureMethod,
                                           const QByteArray &token, const QByteArray &tokenSecret,
                                           const QOAuth::ParamMap &params, QOAuth::ParsingMode mode )
{
  Q_D(QOAuth);

  QOAuth::ParamMap parameters = params;
  QByteArray signature = d->createSignature( requestUrl, httpMethod, signatureMethod,
                                             token, tokenSecret, &parameters );
  parameters.insert( QOAuth::ParamSignature, signature );
  QByteArray parametersString = d->createParametersString( parameters, mode );

  return parametersString;
}
