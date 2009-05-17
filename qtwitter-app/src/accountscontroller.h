#ifndef ACCOUNTSCONTROLLER_H
#define ACCOUNTSCONTROLLER_H

#include <QObject>

class QWidget;
class QModelIndex;
class AccountsModel;
class AccountsView;

namespace Ui {
  class Accounts;
};

class AccountsController : public QObject
{
  Q_OBJECT
public:
  AccountsController( QWidget *widget, QObject *parent );
  virtual ~AccountsController();
  AccountsModel* getModel() const;

public slots:
  void loadAccounts();


private slots:
  void updateAccounts( const QModelIndex &topLeft, const QModelIndex &bottomRight );
  void updateCheckBox( const QModelIndex &index );
  void addAccount();
  void deleteAccount();

private:
  void setAccountEnabled( bool state );
  void setAccountLogin( const QString &login );
  void setAccountPassword( const QString &password );
  void setAccountDM( bool state );
  AccountsModel *model;
  AccountsView *view;
  Ui::Accounts *ui;
};

#endif // ACCOUNTSCONTROLLER_H
