#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>

#include "urlshortenerimplementation.h"

UrlShortenerImplementation::UrlShortenerImplementation( QObject *parent ) : QObject( parent )
{
  connection = new QNetworkAccessManager( this );
  connect( connection, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
}

UrlShortenerImplementation::~UrlShortenerImplementation() {}

int UrlShortenerImplementation::replyStatus( QNetworkReply *reply ) const
{
  return reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
}


IsgdShortener::IsgdShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void IsgdShortener::shorten( const QString &url )
{
  if( QRegExp( "http://is.gd/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://is.gd/api.php?longurl=" + url ) ) );
  }
}

void IsgdShortener::replyFinished( QNetworkReply * reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    case 500: {
        QString message = response.replace("Error: ", "");
        if( message == "The URL entered was not valid." ) {
          emit errorMessage( tr( "The URL entered was not valid.") );
        } else if ( message == "The URL entered was too long." ) {
          emit errorMessage( tr( "The URL entered was too long.") );
        } else if ( message ==  "The address making this request has been blacklisted by Spamhaus (SBL/XBL) or Spamcop." )  {
          emit errorMessage( tr( "The address making this request has been blacklisted by Spamhaus (SBL/XBL) or Spamcop.") );
        } else if ( message == "The URL entered is a potential spam site and is listed on either the SURBL or URIBL blacklist.") {
          emit errorMessage( tr( "The URL entered is a potential spam site and is listed on either the SURBL or URIBL blacklist." ) );
        } else if ( message == "The URL you entered is on the is.gd's blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)." ) {
          emit errorMessage( tr( "The URL you entered is on the is.gd's blacklist (links to URL shortening sites or is.gd itself are disabled to prevent misuse)." ) );
        } else if ( message == "The address making this request has been blocked by is.gd (normally the result of a violation of its terms of use)." ) {
          emit errorMessage( tr( "The address making this request has been blocked by is.gd (normally the result of a violation of its terms of use)." ) );
        }
      }
      break;
    default:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}


TrimShortener::TrimShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void TrimShortener::shorten( const QString &url )
{
  QString newUrl = url.indexOf( "http://" ) > -1 ? url : "http://" + url;

  if( QRegExp( "http://tr.im/" ).indexIn( newUrl ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://api.tr.im/api/trim_simple?url=" + newUrl ) ) );
  }
}

void TrimShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      if( QRegExp( "\\s*" ).exactMatch( response ) ) {
        emit errorMessage( tr( "The URL has been rejected by the tr.im" ) );
      } else {
        emit shortened( response.trimmed() );
      }
      break;
    default:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}


MetamarkShortener::MetamarkShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void MetamarkShortener::shorten( const QString &url )
{
  if( QRegExp( "http://xrl.us/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://metamark.net/api/rest/simple?long_url=" + url ) ) );
  }
}

void MetamarkShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}


TinyurlShortener::TinyurlShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void TinyurlShortener::shorten( const QString &url )
{
  if( QRegExp( "http://tinyurl.com/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://tinyurl.com/api-create.php?url=" + url ) ) );
  }
}

void TinyurlShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}


TinyarrowsShortener::TinyarrowsShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void TinyarrowsShortener::shorten( const QString &url )
{
  if( QRegExp( "http://➡.ws/" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://tinyarro.ws/api-create.php?utfpure=1&url=" + url ) ) );
  }
}

void TinyarrowsShortener::replyFinished( QNetworkReply *reply )
{
  QString response = QString::fromUtf8( reply->readLine() );

  switch( replyStatus( reply ) ) {
    case 200:
      emit shortened( response );
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}


UnuShortener::UnuShortener( QObject *parent ) : UrlShortenerImplementation( parent ) {}

void UnuShortener::shorten( const QString &url )
{
  if( QRegExp( "http://u.nu" ).indexIn( url ) == -1 ) {
    connection->get( QNetworkRequest( QUrl( "http://u.nu/unu-api-simple?url=" + url ) ) );
  }
}

void UnuShortener::replyFinished( QNetworkReply *reply )
{
  QString response = reply->readLine();

  switch( replyStatus( reply ) ) {
    case 200:
      if( response.indexOf( "http://" ) == 0 ) {
        emit shortened( response );
      } else {
        emit errorMessage( tr( "Your URL has been rejected by u.nu" ) );
      }
      break;
    default: case 500:
      emit errorMessage( tr( "An unknown error occurred when shortening your URL." ) );
  }
}
