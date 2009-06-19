#include "oauthwizard.h"
#include "ui_oauthwizard.h"
#include <qoauth/qoauth.h>

#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QDebug>

const QByteArray OAuthWizard::TwitterRequestTokenURL = "http://twitter.com/oauth/request_token";
const QByteArray OAuthWizard::TwitterAccessTokenURL  = "http://twitter.com/oauth/access_token";
const QByteArray OAuthWizard::TwitterAuthorizeURL    = "http://twitter.com/oauth/authorize";

const QByteArray OAuthWizard::ConsumerKey    = "tIFvBZ10xbiOq5p60EcEdA";
const QByteArray OAuthWizard::ConsumerSecret = "5gXfCncW8qEilc3kAw05db2bbfw8RNsDhEAEl9iijdA";

const QByteArray OAuthWizard::ParamCallback      = "oauth_callback";
const QByteArray OAuthWizard::ParamCallbackValue = "oob";
const QByteArray OAuthWizard::ParamVerifier      = "oauth_verifier";
const QByteArray OAuthWizard::ParamScreenName    = "screen_name";


OAuthWizard::OAuthWizard(QWidget *parent) :
    QDialog(parent),
    state( false ),
    screenName( QByteArray() ),
    token( QByteArray() ),
    tokenSecret( QByteArray() ),
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
  connect( m_ui->pinEdit, SIGNAL(textChanged(QString)), this, SLOT(setOkButtonEnabled()) );
}

OAuthWizard::~OAuthWizard()
{
  delete m_ui;
}

QByteArray OAuthWizard::getScreenName() const
{
  if ( screenName.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "screenName is empty, did you finished the authorization procedure?";
  }
  return screenName;
}

QByteArray OAuthWizard::getOAuthKey() const
{
  if ( token.isEmpty() || tokenSecret.isEmpty() ) {
    qWarning() << __PRETTY_FUNCTION__ << "token and/or tokenSecret are empty, did you finished the authorization procedure?";
  }
  return token + "&" + tokenSecret;
}

bool OAuthWizard::authorized() const
{
  return state;
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

void OAuthWizard::setOkButtonEnabled()
{
  if ( m_ui->pinEdit->text().isEmpty() ) {
    m_ui->okButton->setEnabled( false );
  } else {
    m_ui->okButton->setEnabled( true );
  }
}

void OAuthWizard::openUrl()
{
  QOAuth::ParamMap requestToken = qoauth->requestToken( TwitterRequestTokenURL, QOAuth::GET,
                                                        QOAuth::HMAC_SHA1, 10000 );

  token = requestToken.value( QOAuth::ParamToken );
  tokenSecret = requestToken.value( QOAuth::ParamTokenSecret );

  qDebug() << __PRETTY_FUNCTION__ << requestToken;
  QString url = TwitterAuthorizeURL;

  url.append( "?" );
  url.append( "&" + QOAuth::ParamToken + "=" + token );
  url.append( "&" + ParamCallback + "=" + ParamCallbackValue );

  QDesktopServices::openUrl( QUrl( url ) );
  m_ui->allowLabel->hide();
  m_ui->allowButton->hide();
  m_ui->pinLabel->show();
  m_ui->pinEdit->show();
  m_ui->okButton->show();
}

void OAuthWizard::authorize()
{
  QOAuth::ParamMap otherArgs;
  otherArgs.insert( "oauth_verifier", m_ui->pinEdit->text().toAscii() );
  QOAuth::ParamMap accessToken = qoauth->accessToken( TwitterAccessTokenURL, QOAuth::POST, QOAuth::HMAC_SHA1,
                                                      token, tokenSecret, 10000, otherArgs );
//("oauth_token", "14588921-fSLuSles25KfXYRKtJ4cNBPRzWj4XfVvT4RoggkiM")
//("oauth_token_secret", "12LpUz0aAtAUfwWDxCuHcRZGacFaz9EDJU4NWa1Jqvs")
//("screen_name", "ayoy")
//("user_id", "14588921")
  screenName = accessToken.value( ParamScreenName );
  token = accessToken.value( QOAuth::ParamToken );
  tokenSecret = accessToken.value( QOAuth::ParamTokenSecret );
  qDebug() << accessToken;
  state = true;
}
