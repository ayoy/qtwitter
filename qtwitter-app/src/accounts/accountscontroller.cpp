/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#include <QMessageBox>
#include <qticonloader.h>
#include <settings.h>
#include "accountsmodel.h"
#include "accountsview.h"
#include "accountsdelegate.h"
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

  //> freedesktop experiment begin
  ui->addAccountButton->setIcon(QtIconLoader::icon("list-add", QIcon(":/icons/add_48.png")));
  ui->deleteAccountButton->setIcon(QtIconLoader::icon("list-remove", QIcon(":/icons/cancel_48.png")));
  //< freedesktop experiment end

  ui->publicTimelineComboBox->setCurrentIndex( settings.value( "TwitterAccounts/publicTimeline", PT_NONE ).toInt() );
  ui->passwordsCheckBox->setChecked( settings.value( "General/savePasswords", Qt::Unchecked ).toInt() );

  connect( view, SIGNAL(checkBoxClicked(QModelIndex)), this, SLOT(updateCheckBox(QModelIndex)) );
  connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateAccounts(QModelIndex,QModelIndex)) );
  connect( ui->addAccountButton, SIGNAL(clicked()), this, SLOT(addAccount()));
  connect( ui->deleteAccountButton, SIGNAL(clicked()), this, SLOT(deleteAccount()));
  connect( ui->publicTimelineComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePublicTimeline(int)) );
  connect( ui->passwordsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePasswordStoring(int)) );
  connect( ui->disclaimerButton, SIGNAL(clicked()), this, SLOT(showPasswordDisclaimer()) );

  view->setModel( model );
  view->setItemDelegate( new AccountsDelegate( this ) );

  // TODO: WTF?
  view->setColumnWidth( 0, (int)(view->width() * 0.5 ));
  view->setColumnWidth( 1, (int)(view->width() * 0.7 ));
  view->setColumnWidth( 2, (int)(view->width() * 0.8 ));
  view->setColumnWidth( 3, (int)(view->width() * 0.8 ));
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
    model->account(i).network = (TwitterAPI::SocialNetwork) settings.value( QString( "%1/service" ).arg(i), TwitterAPI::SOCIALNETWORK_TWITTER ).toInt();
    model->account(i).login = settings.value( QString( "%1/login" ).arg(i), "" ).toString();
    if ( ui->passwordsCheckBox->isChecked() ) {
      model->account(i).password = settings.pwHash( settings.value( QString( "%1/password" ).arg(i), "" ).toString() );
    }
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
  switch ( topLeft.column() ) {
  case AccountsModel::COL_ENABLED:
    settings.setValue( QString("TwitterAccounts/%1/enabled").arg( topLeft.row() ), topLeft.data() );
    return;
  case AccountsModel::COL_NETWORK:
    settings.setValue( QString("TwitterAccounts/%1/service").arg( topLeft.row() ), topLeft.data( Qt::EditRole ) );
    return;
  case AccountsModel::COL_LOGIN:
    settings.setValue( QString("TwitterAccounts/%1/login").arg( topLeft.row() ), topLeft.data() );
    return;
  case AccountsModel::COL_PASSWORD:
    if ( ui->passwordsCheckBox->isChecked() )
      settings.setValue( QString("TwitterAccounts/%1/password").arg( topLeft.row() ), ConfigFile::pwHash( topLeft.data( Qt::EditRole ).toString() ) );
    return;
  case AccountsModel::COL_DM:
    settings.setValue( QString("TwitterAccounts/%1/directMessages").arg( topLeft.row() ), topLeft.data() );
  default:
    return;
  }
}

void AccountsController::updateCheckBox( const QModelIndex &index )
{
  Account &account = model->account( index.row() );
  if ( index.column() == AccountsModel::COL_ENABLED ) {
    account.isEnabled = !account.isEnabled;
    setAccountEnabled( account.isEnabled );
  } else if ( index.column() == AccountsModel::COL_DM ) {
    account.directMessages = !account.directMessages;
    setAccountDM( account.directMessages );
  }
  view->update( index );
}

void AccountsController::updatePublicTimeline( int state )
{
  settings.setValue( "TwitterAccounts/publicTimeline", state );
}

void AccountsController::togglePasswordStoring( int state )
{
  if ( state == Qt::Checked ) {
    for ( int i = 0; i < model->rowCount(); ++i ) {
      settings.setValue( QString( "TwitterAccounts/%1/password" ).arg(i), ConfigFile::pwHash( model->index( i, AccountsModel::COL_PASSWORD ).data( Qt::EditRole ).toString() ) );
    }
  } else {
    for ( int i = 0; i < model->rowCount(); ++i ) {
      settings.remove( QString( "TwitterAccounts/%1/password" ).arg(i) );
    }
  }
  settings.setValue( "General/savePasswords", state );
}

void AccountsController::showPasswordDisclaimer()
{
  QMessageBox messageBox( QMessageBox::Warning, tr( "Password security" ), tr( "Please note:" ), QMessageBox::Ok );
  messageBox.setInformativeText( tr( "Although passwords are stored as human unreadable data, they can be easily decoded using the application's source code, which is publicly available. You have been warned." ) );
  messageBox.exec();
}

void AccountsController::addAccount()
{
  model->insertRow( model->rowCount() );
  settings.addAccount( model->rowCount() - 1, model->getAccounts().at( model->rowCount() - 1 ) );
  view->setCurrentIndex( model->index( model->rowCount() - 1, 0 ) );
  ui->deleteAccountButton->setEnabled( true );
}

void AccountsController::deleteAccount()
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  int row = view->selectionModel()->currentIndex().row();
  model->removeRow( row );
  settings.deleteAccount( row, model->rowCount() );
  if ( model->rowCount() <= 0 ) {
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
  settings.setValue( QString("TwitterAccounts/%1/enabled").arg( view->currentIndex().row() ), state );
}

void AccountsController::setAccountDM( bool state )
{
  if ( !view->selectionModel()->currentIndex().isValid() )
    return;
  model->account( view->currentIndex().row() ).directMessages = state;
  settings.setValue( QString("TwitterAccounts/%1/directmsgs").arg( view->currentIndex().row() ), state );
}
