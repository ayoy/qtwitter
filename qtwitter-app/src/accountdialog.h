#ifndef ACCOUNTDIALOG_H
#define ACCOUNTDIALOG_H

#include "ui_accountdialog.h"

class TwitterAccount;

class AccountDialog : public QDialog
{
  Q_OBJECT
  public:
    AccountDialog( QWidget *parent = 0);
    AccountDialog( TwitterAccount* account, QWidget *parent = 0);

    void setAccount (TwitterAccount* account);
    TwitterAccount* account() { return m_account; }

  public slots:
    void accept();

  private:
    Ui::Account ui;

    TwitterAccount* m_account;
};

#endif // ACCOUNT_H
