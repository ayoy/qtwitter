#include "accountsmodel.h"
#include "accountsview.h"
#include "accountsdelegate.h"
#include "settings.h"
#include "accountscontroller.h"
#include "ui_accounts.h"

extern ConfigFile settings;

AccountsController::AccountsController( QWidget *widget, QObject *parent ) :
    QObject( parent ),
    model( new AccountsModel( this ) ),
    ui( new Ui::Accounts )
{
  ui->setupUi( widget );
  view = ui->accountsView;

  connect( view, SIGNAL(checkBoxClicked(QModelIndex)), this, SLOT(updateCheckBox(QModelIndex)) );
  connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateAccounts(QModelIndex,QModelIndex)) );
  connect( ui->addAccountButton, SIGNAL(clicked()), this, SLOT(addAccount()));
  connect( ui->deleteAccountButton, SIGNAL(clicked()), this, SLOT(deleteAccount()));

  view->setModel( model );
  view->setItemDelegate( new AccountsDelegate( this ) );

  view->setColumnWidth( 0, (int)(view->width() * 0.5 ));
  view->setColumnWidth( 1, (int)(view->width() * 0.7 ));
  view->setColumnWidth( 2, (int)(view->width() * 0.8 ));
  view->setColumnWidth( 3, (int)(view->width() * 0.7 ));
  view->setColumnWidth( 4, (int)(view->width() * 0.2 ));
}

AccountsController::~AccountsController()
{
  delete ui;
  ui = 0;
}

AccountsModel* AccountsController::getModel() const
{
  return model;
}

void AccountsController::loadAccounts()
{
  model->clear();
  settings.beginGroup( "TwitterAccounts" );
  for ( int i = 0; i < settings.childGroups().count(); i++ ) {
    model->insertRow(i);
    model->account(i).isEnabled = settings.value( QString( "%1/enabled" ).arg(i), false ).toBool();
    // TODO: provide a static const QString for "Twitter" and "Identi.ca"
    model->account(i).network = Account::networkFromString( settings.value( QString( "%1/service" ).arg(i), "Twitter" ).toString() );
    model->account(i).login = settings.value( QString( "%1/login" ).arg(i), "" ).toString();
    model->account(i).password = settings.pwHash( settings.value( QString( "%1/password" ).arg(i), "" ).toString() );
    model->account(i).directMessages = settings.value( QString( "%1/directmsgs" ).arg(i), false ).toBool();
  }
  settings.endGroup();
  if ( view->model()->rowCount() <= 0 ) {
    ui->deleteAccountButton->setEnabled( false );
  } else {
    ui->deleteAccountButton->setEnabled( true );
  }
}

void AccountsController::updateAccounts( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  // TODO: change config file to organise accounts in an array,
  //       i.e. "TwitterAccounts/%1/%2" with respect to view's row and column
  Q_UNUSED(bottomRight);
  if ( !topLeft.isValid() )
    return;
//  updateAccountsOnExit = true;
  // TODO: consider moving this to AccountsModel::setData()
  switch ( topLeft.column() ) {
  case 0:
    settings.setValue( QString("TwitterAccounts/%1/enabled").arg( topLeft.row() ), topLeft.data() );
    return;
  case 1:
    settings.setValue( QString("TwitterAccounts/%1/service").arg( topLeft.row() ), topLeft.data() );
    return;
  case 2:
    settings.setValue( QString("TwitterAccounts/%1/login").arg( topLeft.row() ), topLeft.data() );
    return;
  case 3:
    settings.setValue( QString("TwitterAccounts/%1/password").arg( topLeft.row() ), topLeft.data( Qt::EditRole ) );
    return;
  case 4:
    settings.setValue( QString("TwitterAccounts/%1/directMessages").arg( topLeft.row() ), topLeft.data() );
  default:
    return;
  }
}

void AccountsController::updateCheckBox( const QModelIndex &index )
{
  Account &account = model->account( index.row() );
  if ( index.column() == 0 ) {
    account.isEnabled = !account.isEnabled;
    setAccountEnabled( account.isEnabled );
  } else if ( index.column() == 4 ) {
    account.directMessages = !account.directMessages;
    setAccountDM( account.directMessages );
  }
  view->update( index );
//  updateAccountsOnExit = true;
}

void AccountsController::addAccount()
{
  model->insertRow( model->rowCount() );
  view->setCurrentIndex( model->index( model->rowCount() - 1, 0 ) );
  ui->deleteAccountButton->setEnabled( true );
  settings.beginGroup( QString( "TwitterAccounts/%1" ).arg( model->rowCount() - 1 ) );
    settings.setValue( "enabled", true );
    settings.setValue( "service", TwitterAPI::SOCIALNETWORK_TWITTER );
    //: This is for newly created account - when the login isn't given yet
    settings.setValue( "login", tr( "<empty>" ) );
    settings.setValue( "password", "" );
    settings.setValue( "directmsgs", false );
  settings.endGroup();
}

void AccountsController::deleteAccount()
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  int row = view->selectionModel()->currentIndex().row();
  model->removeRow( row );
  settings.deleteAccount( row, model->rowCount() );
  if ( model->rowCount() <= 1 ) {
    ui->deleteAccountButton->setEnabled( false );
  } else {
    ui->deleteAccountButton->setEnabled( true );
  }
}

void AccountsController::setAccountEnabled( bool state )
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  model->account( view->currentIndex().row() ).isEnabled = state;
  view->update( model->index( view->currentIndex().row(), 0, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/enabled").arg( view->currentIndex().row() ), state );
}

void AccountsController::setAccountLogin( const QString &login )
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  Account &account = model->account( view->currentIndex().row() );
  account.login = login;
  view->update( model->index( view->currentIndex().row(), 1, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/login").arg( view->currentIndex().row() ), login );
}

void AccountsController::setAccountPassword( const QString &password )
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  Account &account = model->account( view->currentIndex().row() );
  account.password = password;
  settings.setValue( QString("TwitterAccounts/%1/password").arg( view->currentIndex().row() ), settings.pwHash( password ) );
}

void AccountsController::setAccountDM( bool state )
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  Account &account = model->account( view->currentIndex().row() );
  account.directMessages = state;
  view->update( model->index( view->currentIndex().row(), 3, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/directmsgs").arg( view->currentIndex().row() ), state );
}
