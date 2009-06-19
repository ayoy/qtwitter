#include "oauthwizard.h"
#include "ui_oauthwizard.h"
#include <qoauth/qoauth.h>

//#include <QNetworkAccessManager>
//#include <QNetworkRequest>
//#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDebug>

const QByteArray OAuthWizard::TwitterRequestTokenURL = "http://twitter.com/oauth/request_token";
const QByteArray OAuthWizard::TwitterAccessTokenURL  = "http://twitter.com/oauth/access_token";
const QByteArray OAuthWizard::TwitterAuthorizeURL    = "http://twitter.com/oauth/authorize";

const QByteArray OAuthWizard::ConsumerKey    = "tIFvBZ10xbiOq5p60EcEdA";
const QByteArray OAuthWizard::ConsumerSecret = "5gXfCncW8qEilc3kAw05db2bbfw8RNsDhEAEl9iijdA";

OAuthWizard::OAuthWizard(QWidget *parent) :
    QDialog(parent),
    qoauth( new QOAuth( this ) ),
    m_ui(new Ui::OAuthWizard)
{
  qoauth->setConsumerKey( ConsumerKey );
  qoauth->setConsumerSecret( ConsumerSecret );

  m_ui->setupUi(this);
  m_ui->pinLabel->hide();
  m_ui->pinEdit->hide();
  m_ui->okButton->hide();
  connect( m_ui->allowButton, SIGNAL(clicked()), this, SLOT(openUrl()) );
  connect( m_ui->okButton, SIGNAL(clicked()), this, SLOT(authorize()) );
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
  QByteArray tokenString = qoauth->requestToken( TwitterRequestTokenURL,
                                                 QOAuth::HMAC_SHA1, QOAuth::GET, 10000 );
  qDebug() << __PRETTY_FUNCTION__ << tokenString;
  QString url = TwitterAuthorizeURL;

  url.append( "?" );
  url.append( tokenString );
  url.append( "&oauth_callback=oob" );

  QDesktopServices::openUrl( QUrl( url ) );
  m_ui->allowLabel->hide();
  m_ui->allowButton->hide();
  m_ui->pinLabel->show();
  m_ui->pinEdit->show();
  m_ui->okButton->show();
}

void OAuthWizard::authorize()
{
  QMap<QByteArray,QByteArray> otherArgs;
  otherArgs.insert( "oauth_verifier", m_ui->pinEdit->text().toAscii() );
  QByteArray accessToken = qoauth->accessToken( TwitterAccessTokenURL, QOAuth::HMAC_SHA1, QOAuth::POST, 10000, otherArgs );
}
