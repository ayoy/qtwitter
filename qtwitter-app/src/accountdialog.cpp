#include "accountdialog.h"
#include "twitteraccountsmodel.h"

AccountDialog::AccountDialog( QWidget *parent)
  : QDialog( parent )
{
  ui.setupUi( this );

  connect (ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect (ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AccountDialog::AccountDialog( TwitterAccount* account, QWidget *parent)
  : QDialog( parent )
{
  ui.setupUi( this );

  setAccount(account);

  connect (ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect (ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void AccountDialog::setAccount (TwitterAccount* account)
{
  m_account = account;
  ui.accountDMCheckBox->setChecked(m_account->directMessages);
  ui.accountEnabledCheckBox->setChecked(m_account->isEnabled);
  ui.accountLoginEdit->setText(m_account->login);
  ui.accountPasswordEdit->setText(m_account->password);
}


void AccountDialog::accept()
{
  m_account->login = ui.accountLoginEdit->text();
  m_account->password = ui.accountPasswordEdit->text();
  m_account->isEnabled = ui.accountEnabledCheckBox->isChecked();
  m_account->directMessages = ui.accountDMCheckBox->isChecked();
  QDialog::accept();
}


