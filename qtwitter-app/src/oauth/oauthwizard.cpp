#include "oauthwizard.h"
#include "ui_oauthwizard.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDateTime>
#include <QDebug>

OAuthWizard::OAuthWizard(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::OAuthWizard)
{
  m_ui->setupUi(this);
  manager = new QNetworkAccessManager( this );
  connect( m_ui->pushButton, SIGNAL(clicked()), this, SLOT(openUrl()) );
  connect( manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)) );
}

OAuthWizard::~OAuthWizard()
{
  delete m_ui;
}

void OAuthWizard::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void OAuthWizard::openUrl()
{
  uint time = QDateTime::currentDateTime().toTime_t();
  QString timestamp = QString::number( time );
  QString signatureString( "GET"
                           "&http://twitter.com/oauth/request_token"
                           "&content-type=application/x-www-form-urlencoded"
                           "&oauth_consumer_key=tIFvBZ10xbiOq5p60EcEdA"
                           "&oauth_nonce=4572616e48616d6d65724c61686176"
                           "&oauth_signature_method=PLAINTEXT"
                           "&oauth_timestamp=%1"
                           "&oauth_token=&oauth_version=1.0" );
  signatureString.arg( timestamp );

  QByteArray signature = signatureString.toAscii().toPercentEncoding();

  QByteArray authorizationHeader( "OAuth realm=\"http://twitter.com/\","
                                  "oauth_consumer_key=\"tIFvBZ10xbiOq5p60EcEdA\","
                                  "oauth_token=\"\","
                                  "oauth_signature_method=\"PLAINTEXT\","
                                  "oauth_signature=\"" );
  authorizationHeader.append( signature );
  authorizationHeader.append( "\","
                              "oauth_timestamp=\"" );
  authorizationHeader.append( timestamp );
  authorizationHeader.append( "\","
                              "oauth_nonce=\"4572616e48616d6d65724c61686176\","
                              "oauth_version=\"1.0\"" );

  QNetworkRequest request( QUrl( "http://twitter.com/oauth/request_token" ) );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
  request.setRawHeader( "Authorization", authorizationHeader );



}

void OAuthWizard::requestFinished( QNetworkReply *reply )
{
  int code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
  if ( code != 200 ) {
    qDebug() << "Error:" << code;
  } else {
    qDebug() << reply->readAll();
  }
}
