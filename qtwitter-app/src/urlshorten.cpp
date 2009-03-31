#include "urlshorten.h"
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QRegExp>

UrlShorten::UrlShorten( QObject *parent ) : QObject( parent )
{
    manager = new QNetworkAccessManager( this );
    connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)) );
    connect( this, SIGNAL(errorMessage(QString)), parent, SIGNAL(errorMessage(QString)) );
}

UrlShorten::~UrlShorten()
{
}

void UrlShorten::shorten( const QString &url )
{
  QRegExp rx("http://is.gd/");
  if( rx.indexIn( url ) == -1 ){
    manager->get( QNetworkRequest( QUrl( "http://is.gd/api.php?longurl=" + url ) ) );
  }
}

void UrlShorten::replyFinished( QNetworkReply * reply )
{
  int status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  QString response = reply->readLine();

  switch( status ) {
    case 200:
      emit shortened( response );
      break;
    case 500:
      emit errorMessage( response.replace("Error: ", "") );
      break;
    default:
      emit errorMessage( "An unknown error occurred when shortening your URL" );
  }
}

/*! \class UrlShorten
    \brief A class responsible for interacting with URL shortening services.

    This class provides an interface for communicating with URL shortening services.
    Right now, it only supports http://is.gd
*/

/*! \fn UrlShorten::UrlShorten( QObject *parent )
    Creates a new instance of UrlShorten class with the given \a parent
*/

/*! \fn UrlShorten~UrlShorten()
    Destroys UrlShorten instance
*/

/*! \fn void UrlShorten::shorten( const QString &url )
    Sends a request to the shortening service with the give \a url
*/

/*! \fn void UrlShorten::shortened( const QString &url )
    Emitted for a shortened URL
*/

