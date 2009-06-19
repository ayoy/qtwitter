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

const QByteArray QOAuthPrivate::ParamConsumerKey     = "oauth_consumer_key";
const QByteArray QOAuthPrivate::ParamNonce           = "oauth_nonce";
const QByteArray QOAuthPrivate::ParamSignature       = "oauth_signature";
const QByteArray QOAuthPrivate::ParamSignatureMethod = "oauth_signature_method";
const QByteArray QOAuthPrivate::ParamTimestamp       = "oauth_timestamp";
const QByteArray QOAuthPrivate::ParamVersion         = "oauth_version";
const QByteArray QOAuthPrivate::ParamToken           = "oauth_token";
const QByteArray QOAuthPrivate::ParamTokenSecret     = "oauth_token_secret";
const QByteArray QOAuthPrivate::ParamAccessToken     = "oauth_access_token";

QOAuthPrivate::QOAuthPrivate( QObject *parent ) :
    QObject( parent ),
    consumerKey( QByteArray() ),
    consumerSecret( QByteArray() ),
    token( QByteArray() ),
    tokenSecret( QByteArray() ),
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

QByteArray QOAuthPrivate::createParametersString( const QMap<QByteArray,QByteArray> &parameters, ParsingMode mode )
{
  QByteArray middleString;
  QByteArray endString;
  switch ( mode ) {
  // equals to ParseForInlineQuery
  case ParseForSignatureBaseString:
    middleString = "=";
    endString = "&";
    break;
  case ParseForHeaderArguments:
    middleString = "=\"";
    endString = "\",";
    break;
  default:
    qWarning() << __PRETTY_FUNCTION__ << "Unrecognized mode";
    return QByteArray();
  }

  QByteArray parameter;
  QByteArray parametersString;

  foreach( parameter, parameters.keys() ) {
    parametersString.append( parameter );
    parametersString.append( middleString );
    parametersString.append( parameters.value( parameter ) );
    parametersString.append( endString );
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
      qDebug() << reply->readAll();
      break;
    default:;
    }

    break;
  case 401:
  case 403:

  default:;
  }
}

bool QOAuthPrivate::parseRequestTokenReply( const QByteArray &data )
{
  // split reply to name=value strings
  QList<QByteArray> replyParams = data.split( '&' );
  // we'll store them in a map
  QMap<QByteArray,QByteArray> parameters;

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
    parameters[ key ] = replyParam.right( replyParam.length() - separatorIndex - 1 );
  }

  if ( !parameters.contains( QOAuthPrivate::ParamToken ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token not present in reply!";
    return false;
  }
  if ( !parameters.contains( QOAuthPrivate::ParamTokenSecret ) ) {
    qWarning() << __PRETTY_FUNCTION__ << "oauth_token_secret not present in reply!";
    return false;
  }
  // assign token and tokenSecret values
  token = parameters.value( QOAuthPrivate::ParamToken );
  tokenSecret = parameters.value( QOAuthPrivate::ParamTokenSecret );
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

QByteArray QOAuth::token() const
{
  Q_D(const QOAuth);

  return d->token;
}

QByteArray QOAuthPrivate::createSignature( const QString &requestUrl, QOAuth::SignatureMethod signatureMethod,
                            QOAuth::HttpMethod httpMethod, const QOAuth::MiscParams &params )
{
  return QByteArray();
}


QByteArray QOAuth::requestToken( const QString &requestUrl, SignatureMethod signatureMethod, HttpMethod httpMethod,
                                 uint timeout, const MiscParams &params )
{
  Q_D(QOAuth);

  if ( d->consumerKey.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer key is empty, make sure that you set it with QOAuth::setConsumerKey()";
    return QByteArray();
  }
  if ( d->consumerSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer secret is empty, make sure that you set it with QOAuth::setConsumerSecret()";
    return QByteArray();
  }

  // temporarily only HMAC-SHA1 is supported
  if ( signatureMethod != HMAC_SHA1 ) {
    qWarning() << __PRETTY_FUNCTION__ << "Sorry, we're currently supporting only HMAC-SHA1 method...";
    return QByteArray();
  }

//  d->createSignature( requestUrl, signatureMethod, httpMethod, params );

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
  QByteArray httpMethodString = d->httpMethodToString( httpMethod );
  // 2. prepare percent-encoded request URL
  QByteArray percentRequestUrl = requestUrl.toAscii().toPercentEncoding();
  // 3. prepare percent-encoded parameters string
  QMap<QByteArray,QByteArray> parameters;
  parameters.insert( QOAuthPrivate::ParamConsumerKey, d->consumerKey );
  parameters.insert( QOAuthPrivate::ParamNonce, nonce );
  parameters.insert( QOAuthPrivate::ParamSignatureMethod,
                     d->signatureMethodToString( signatureMethod ) );
  parameters.insert( QOAuthPrivate::ParamTimestamp, timestamp );
  parameters.insert( QOAuthPrivate::ParamVersion, Version );
  parameters = parameters.unite( params );

  QByteArray parametersString = d->createParametersString( parameters, QOAuthPrivate::ParseForSignatureBaseString );
  QByteArray percentParametersString = parametersString.toPercentEncoding();

  // 4. create signature base string
  QByteArray signatureBaseString;
  signatureBaseString.append( httpMethodString + "&" );
  signatureBaseString.append( percentRequestUrl + "&" );
  signatureBaseString.append( percentParametersString );

  // create key for HMAC-SHA1 hashing
  QByteArray key( d->consumerSecret + "&" );

  // create HMAC-SHA1 digest in Base64
  QCA::MessageAuthenticationCode hmac( "hmac(sha1)", QCA::SymmetricKey( key ) );
  QCA::SecureArray array( signatureBaseString );
  hmac.update( array );
  QCA::SecureArray resultArray = hmac.final();
  QByteArray digest = resultArray.toByteArray().toBase64();
  // percent-encode the digest
  QByteArray signature = digest.toPercentEncoding();

  // add signature to parameters and create authorization header
  parameters.insert( QOAuthPrivate::ParamSignature, signature );
  QByteArray authorizationHeader = d->createParametersString( parameters, QOAuthPrivate::ParseForHeaderArguments );

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
//  qDebug() << "token:" << d->token
//           << "tokenSecret:" << d->tokenSecret;

  // prepare the result string
  QByteArray result( QOAuthPrivate::ParamToken );
  result.append( "=" + d->token );

  return result;
}

//void QOAuth::authorize()
//{
//}

void QOAuth::authenticate()
{
}

QByteArray QOAuth::accessToken( const QString &requestUrl, SignatureMethod signatureMethod, HttpMethod httpMethod,
                          uint timeout, const MiscParams &params )
{
  Q_D(QOAuth);

  if ( d->consumerKey.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer key is empty, make sure that you set it with QOAuth::setConsumerKey()";
    return QByteArray();
  }
  if ( d->consumerSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "consumer secret is empty, make sure that you set it with QOAuth::setConsumerSecret()";
    return QByteArray();
  }
  if ( d->token.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "token is empty, make sure that you obtained it with QOAuth::requestToken()";
    return QByteArray();
  }
  if ( d->tokenSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "token secret is empty, make sure that you obtained it with QOAuth::requestToken()";
    return QByteArray();
  }


  // temporarily only HMAC-SHA1 is supported
  if ( signatureMethod != HMAC_SHA1 ) {
    qWarning() << __PRETTY_FUNCTION__ << "Sorry, we're currently supporting only HMAC-SHA1 method...";
    return QByteArray();
  }

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
  QByteArray httpMethodString = d->httpMethodToString( httpMethod );
  // 2. prepare percent-encoded request URL
  QByteArray percentRequestUrl = requestUrl.toAscii().toPercentEncoding();
  // 3. prepare percent-encoded parameters string
  QMap<QByteArray,QByteArray> parameters;
  parameters.insert( QOAuthPrivate::ParamConsumerKey, d->consumerKey );
  parameters.insert( QOAuthPrivate::ParamNonce, nonce );
  parameters.insert( QOAuthPrivate::ParamSignatureMethod,
                     d->signatureMethodToString( signatureMethod ) );
  parameters.insert( QOAuthPrivate::ParamTimestamp, timestamp );
  parameters.insert( QOAuthPrivate::ParamVersion, Version );
  parameters.insert( QOAuthPrivate::ParamToken, d->token );
  parameters = parameters.unite( params );

  QByteArray parametersString = d->createParametersString( parameters, QOAuthPrivate::ParseForSignatureBaseString );
  QByteArray percentParametersString = parametersString.toPercentEncoding();

  // 4. create signature base string
  QByteArray signatureBaseString;
  signatureBaseString.append( httpMethodString + "&" );
  signatureBaseString.append( percentRequestUrl + "&" );
  signatureBaseString.append( percentParametersString );

  // create key for HMAC-SHA1 hashing
  QByteArray key( d->consumerSecret + "&" + d->tokenSecret );

  // create HMAC-SHA1 digest in Base64
  QCA::MessageAuthenticationCode hmac( "hmac(sha1)", QCA::SymmetricKey( key ) );
  QCA::SecureArray array( signatureBaseString );
  hmac.update( array );
  QCA::SecureArray resultArray = hmac.final();
  QByteArray digest = resultArray.toByteArray().toBase64();
  // percent-encode the digest
  QByteArray signature = digest.toPercentEncoding();

  // add signature to parameters and create authorization header
  parameters.insert( QOAuthPrivate::ParamSignature, signature );
  QByteArray authorizationHeader = d->createParametersString( parameters, QOAuthPrivate::ParseForInlineQuery );

  // create a network request
  QNetworkRequest request;
//  request.setRawHeader( "Authorization", "OAuth " + authorizationHeader );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );

  request.setUrl( QUrl( requestUrl ) );
  request.setAttribute( QOAuthPrivate::RequestType, QOAuthPrivate::AccessToken );

  qDebug() << QString( "curl -i -d '%1' %2" ).arg( QString(authorizationHeader) ).arg( request.url().toString() );
//  qDebug() << QString( "curl -i -H 'Authorization: %1' \"%2\"" ).arg( request.rawHeader( "Authorization" ), request.url().toString() );
  // fire up a single shot timer if timeout was specified
  if ( timeout > 0 ) {
    QTimer::singleShot( timeout, d->loop, SLOT(quit()) );
  }
  // send the request
  d->manager->post( request, authorizationHeader );
  // start the event loop and wait for the response
  d->loop->exec();
//  qDebug() << "token:" << d->token
//           << "tokenSecret:" << d->tokenSecret;

  // prepare the result string
  QByteArray result( QOAuthPrivate::ParamAccessToken );
  result.append( "=" + d->accessToken );

  return result;
}
