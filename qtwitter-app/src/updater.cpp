#include "updater.h"
#include "configfile.h"

#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QDebug>

Updater::Updater( QObject *parent ) :
    QNetworkAccessManager( parent )
{
  connect( this, SIGNAL(finished(QNetworkReply*)), SLOT(readReply(QNetworkReply*)) );
}

Updater::~Updater()
{
}

void Updater::checkForUpdate()
{
  QByteArray data( "version=" );
  data.append( ConfigFile::APP_VERSION );
  qDebug() << "checking for updates: current version is" << ConfigFile::APP_VERSION;
  post( QNetworkRequest( QUrl( "http://qtwitter.ayoy.net/cgi-bin/latest2" ) ), data );
}

void Updater::readReply( QNetworkReply *reply )
{
  int returnCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  qDebug() << returnCode;
  if ( returnCode == 200 ) {
    QString latest = reply->readLine();
    latest.chop(1); // remove the trailing enter
    QString changes = reply->readAll();
    bool available = !latest.isEmpty();
    emit updateChecked( available, available ? latest : ConfigFile::APP_VERSION, available ? changes : QString() );
  }
}
