#ifndef OAUTHWIZARD_H
#define OAUTHWIZARD_H

#include <QtGui/QDialog>

namespace Ui {
    class OAuthWizard;
}

class QNetworkAccessManager;
class QNetworkReply;

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
    void requestFinished( QNetworkReply *reply );

private:
//    static const QString key;
    Ui::OAuthWizard *m_ui;
    QNetworkAccessManager *manager;
};

#endif // OAUTHWIZARD_H
