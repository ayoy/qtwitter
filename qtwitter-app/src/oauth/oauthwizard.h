#ifndef OAUTHWIZARD_H
#define OAUTHWIZARD_H

#include <QtGui/QDialog>

namespace Ui {
  class OAuthWizard;
}

class QOAuth;

class OAuthWizard : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY(OAuthWizard)

public:
  explicit OAuthWizard(QWidget *parent = 0);
  virtual ~OAuthWizard();

protected:
  virtual void changeEvent(QEvent *e);

private slots:
  void openUrl();
  void authorize();

private:
  QOAuth *qoauth;
  Ui::OAuthWizard *m_ui;

  static const QByteArray TwitterRequestTokenURL;
  static const QByteArray TwitterAccessTokenURL;
  static const QByteArray TwitterAuthorizeURL;
  static const QByteArray ConsumerKey;
  static const QByteArray ConsumerSecret;
};

#endif // OAUTHWIZARD_H
