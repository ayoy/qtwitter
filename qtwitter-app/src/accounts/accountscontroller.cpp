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


#include "accountscontroller.h"
#include "ui_accounts.h"

#include "accountsmodel.h"
#include "accountsview.h"
#include "accountsdelegate.h"
#include "newaccountdialog.h"
#include <configfile.h>
#include <qticonloader.h>

#ifdef OAUTH
#  include <oauthwizard.h>
#endif

#include <QMessageBox>
#include <QInputDialog>
#include <QModelIndex>
#include <QDebug>

extern ConfigFile settings;

AccountsController::AccountsController( QWidget *widget, QObject *parent ) :
    QObject( parent ),
    model( 0 ),
    ui( new Ui::Accounts ),
    widget( widget )
{
  ui->setupUi( widget );
  view = ui->accountsView;

  //> freedesktop experiment begin
  ui->addAccountButton->setIcon(QtIconLoader::icon("list-add", QIcon(":/icons/add_48.png")));
  ui->deleteAccountButton->setIcon(QtIconLoader::icon("list-remove", QIcon(":/icons/cancel_48.png")));
  //< freedesktop experiment end

  ui->passwordsCheckBox->setChecked( settings.value( "General/savePasswords", Qt::Checked ).toInt() );

  connect( view, SIGNAL(checkBoxClicked(QModelIndex)), this, SLOT(updateCheckBox(QModelIndex)) );
  connect( ui->addAccountButton, SIGNAL(clicked()), this, SLOT(addAccount()));
  connect( ui->deleteAccountButton, SIGNAL(clicked()), this, SLOT(deleteAccount()));
  connect( ui->passwordsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePasswordStoring(int)) );
  connect( ui->disclaimerButton, SIGNAL(clicked()), this, SLOT(showPasswordDisclaimer()) );

  view->setItemDelegate( new AccountsDelegate( this ) );
}

AccountsController::~AccountsController()
{
  updateAccounts( model->index(0,0), model->index( model->rowCount() - 1, model->columnCount() - 1 ) );
  delete ui;
  ui = 0;
}

AccountsModel* AccountsController::getModel() const
{
  return model;
}

void AccountsController::setModel( AccountsModel *accountsModel )
{
  model = accountsModel;
  model->setParent( this );
  view->setModel( model );
  connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateAccounts(QModelIndex,QModelIndex)) );
  // TODO: WTF?
  view->setColumnWidth( 0, (int)(view->width() * 0.5 ));
  view->setColumnWidth( 1, (int)(view->width() * 0.7 ));
  view->setColumnWidth( 2, (int)(view->width() * 0.8 ));
  view->setColumnWidth( 3, (int)(view->width() * 0.8 ));
  view->setColumnWidth( 4, (int)(view->width() * 0.2 ));
}

void AccountsController::loadAccounts()
{
  if (!model)
    return;

  settings.beginGroup( "Accounts" );
  int accountsCount = settings.childGroups().count();

  QList<Account> modelAccounts = model->getAccounts();
  QList<Account> settingsAccounts;
  QList<QString> passwords;
  for ( int i = 0; i < accountsCount; i++ ) {
    Account account;
    QString id = QString::number(i);
    account.isEnabled = settings.value( QString( "%1/enabled" ).arg(id), false ).toBool();
    account.network = (TwitterAPI::SocialNetwork) settings.value( QString( "%1/service" ).arg(id), TwitterAPI::SOCIALNETWORK_TWITTER ).toInt();
    account.login = settings.value( QString( "%1/login" ).arg(id), "" ).toString();
    account.directMessages = settings.value( QString( "%1/directmsgs" ).arg(id), false ).toBool();

    settingsAccounts << account;
    account.password = settings.pwHash( settings.value( QString( "%1/password" ).arg(id), "" ).toString() );
    passwords << account.password;
  }

  for( int i = 0; i < modelAccounts.size(); ++i ) {
    Account &modelAccount = modelAccounts[i];
    if ( !settingsAccounts.contains( modelAccount ) ) {
      modelAccounts.removeAll( modelAccount );
    } else {
      if ( ui->passwordsCheckBox->isChecked()
#ifdef OAUTH
        || ( !ui->passwordsCheckBox->isChecked() &&
             modelAccount.network == TwitterAPI::SOCIALNETWORK_TWITTER )
#endif
        ) {
        int modelAccountIndex = settingsAccounts.indexOf( modelAccount );
        modelAccount.password = passwords.at( modelAccountIndex );
        passwords.removeAt( modelAccountIndex );
        settingsAccounts.removeAt( modelAccountIndex );
//        qDebug() << __FUNCTION__ << "MODEL" << modelAccount.login << modelAccount.password;
      }
    }
  }

  for( int i = 0; i < settingsAccounts.size(); ++i ) {
    Account &settingsAccount = settingsAccounts[i];
    if ( !modelAccounts.contains( settingsAccount ) ) {
      if ( ui->passwordsCheckBox->isChecked()
#ifdef OAUTH
        || ( !ui->passwordsCheckBox->isChecked() &&
             settingsAccount.network == TwitterAPI::SOCIALNETWORK_TWITTER )
#endif
        ) {
        settingsAccount.password = passwords.at( settingsAccounts.indexOf( settingsAccount ) );
//        qDebug() << __FUNCTION__ << "SETTINGS" << settingsAccount.login << settingsAccount.password;
        modelAccounts.insert( i, settingsAccount );
      }
    } else {
      int j = modelAccounts.indexOf( settingsAccount );
      modelAccounts.move(j, i);
    }
  }

  // TODO: Probably doesn't work
  qSort( modelAccounts );

  model->setAccounts( modelAccounts );
  settings.endGroup(); //Accounts

  updateAccounts( model->index(0,0), model->index( model->rowCount() - 1, model->columnCount() - 1 ) );

  if ( view->model()->rowCount() <= 0 ) {
    ui->deleteAccountButton->setEnabled( false );
  } else {
    ui->deleteAccountButton->setEnabled( true );
  }
}

void AccountsController::updateAccounts( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  // TODO: change config file to organise accounts in an array,
  //       i.e. "Accounts/%1/%2" with respect to view's row and column

//  updateAccountsOnExit = true;

  for( int i = topLeft.row(); i <= bottomRight.row(); ++i )
    for (int j = topLeft.column(); j <= bottomRight.column(); ++j ) {

    if ( model->index(i, j).isValid() ) {
      switch ( j ) {
      case AccountsModel::COL_ENABLED:
        settings.setValue( QString("Accounts/%1/enabled").arg( i ), model->index(i,j).data( Qt::CheckStateRole ) != Qt::Unchecked );
        break;
      case AccountsModel::COL_NETWORK:
        settings.setValue( QString("Accounts/%1/service").arg( i ), model->index(i,j).data( Qt::EditRole ) );
        break;
      case AccountsModel::COL_LOGIN:
        settings.setValue( QString("Accounts/%1/login").arg( i ), model->index(i,j).data() );
        break;
      case AccountsModel::COL_PASSWORD:
        if ( ui->passwordsCheckBox->isChecked()
#ifdef OAUTH
             || model->index(i, AccountsModel::COL_NETWORK ).data( Qt::EditRole ) == TwitterAPI::SOCIALNETWORK_TWITTER
#endif
           ) {
          settings.setValue( QString("Accounts/%1/password").arg( i ), ConfigFile::pwHash( model->index(i,j).data( Qt::EditRole ).toString() ) );
        } else {
          settings.setValue( QString("Accounts/%1/password").arg( i ), QString() );
        }
        break;
      case AccountsModel::COL_DM:
        settings.setValue( QString("Accounts/%1/directmsgs").arg( i ), model->index(i,j).data( Qt::CheckStateRole ) != Qt::Unchecked );
      default:
        break;
      }
    }
  }
}

void AccountsController::updateCheckBox( const QModelIndex &index )
{
  if (!model)
    return;

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

void AccountsController::togglePasswordStoring( int state )
{
  if (!model)
    return;

  if ( state == Qt::Checked ) {
    for ( int i = 0; i < model->rowCount(); ++i ) {
      settings.setValue( QString( "Accounts/%1/password" ).arg(i), ConfigFile::pwHash( model->index( i, AccountsModel::COL_PASSWORD ).data( Qt::EditRole ).toString() ) );
    }
  } else {
    for ( int i = 0; i < model->rowCount(); ++i ) {
#ifdef OAUTH
      if ( model->account(i).network == TwitterAPI::SOCIALNETWORK_IDENTICA )
#endif
        settings.remove( QString( "Accounts/%1/password" ).arg(i) );
    }
  }
  settings.setValue( "General/savePasswords", state );
}

void AccountsController::showPasswordDisclaimer()
{
  QMessageBox messageBox( QMessageBox::Warning, tr( "Password security" ), tr( "Please note:" ), QMessageBox::Ok );
  messageBox.setInformativeText( tr( "Although passwords are stored as human unreadable data, "
                                     "they can be easily decoded using the application's source code, "
                                     "which is publicly available. You have been warned." ) );
#ifdef OAUTH
  messageBox.setInformativeText( messageBox.informativeText().append( "<br><br>" )
                                 .append( tr( "Note also that Twitter authorization keys are stored anyway. "
                                              "Remove the account from the list if you want the key to be deleted."
                                              /*"They can't be reused outside this application."*/ ) ) );
#endif
  messageBox.exec();
}

void AccountsController::addAccount()
{
  if (!model)
    return;

  int result;
  int network;
  QString login;
  QString password;

  if ( sender() == ui->addAccountButton ) {
#if QT_VERSION < 0x040500
    bool ok = false;
    QString network = QInputDialog::getItem( view, tr( "Add account" ), tr( "Select social network:" ),
                                             QStringList() << "Twitter" << "Identi.ca", 0, false, &ok );
    int result = ok ? QDialog::Accepted : QDialog::Rejected;
#else
    QInputDialog *dlg = new QInputDialog( view );
    dlg->setWindowTitle( tr( "Add account" ) );
    //: Select social network, i.e. Twitter or Identi.ca
    dlg->setLabelText( tr( "Select social network:" ) );
    dlg->setComboBoxItems( QStringList() << "Twitter" << "Identi.ca" );
    dlg->setCancelButtonText( tr( "Cancel" ) );
    dlg->setOkButtonText( tr( "OK" ) );
    result = dlg->exec();
    network = dlg->textValue() == "Twitter" ? TwitterAPI::SOCIALNETWORK_TWITTER : TwitterAPI::SOCIALNETWORK_IDENTICA;
    dlg->deleteLater();
#endif
  } else {
    NewAccountDialog dlg;
    result = dlg.exec();
    network = dlg.network();
    login = dlg.login();
    password = dlg.password();
  }

  if ( result == QDialog::Accepted ) {
    int index = model->rowCount();

#ifdef OAUTH
    if ( network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
      OAuthWizard *wizard = new OAuthWizard( view );
      wizard->exec();
      if ( wizard->authorized() ) {
        model->insertRow( index );

        Account &account = model->account( index );
        account.network = TwitterAPI::SOCIALNETWORK_TWITTER;
        account.login = wizard->getScreenName();
        account.password = wizard->getOAuthKey();
        settings.addAccount( index, model->account( index ) );
        view->setCurrentIndex( model->index( index, 0 ) );
        ui->deleteAccountButton->setEnabled( true );
        emit accountDialogClosed( true );
      } else {
        emit accountDialogClosed( false );
      }
      wizard->deleteLater();

    } else if ( network == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
#endif
      model->insertRow( index );
      model->account( index ).network = (TwitterAPI::SocialNetwork) network;
      if ( sender() != ui->addAccountButton ) {
        model->account( index ).login = login;
        model->account( index ).password = password;
      }
      settings.addAccount( index, model->account( index ) );
      view->setCurrentIndex( model->index( index, 0 ) );
      ui->deleteAccountButton->setEnabled( true );
      emit accountDialogClosed( true );
#ifdef OAUTH
    }
#endif
  } else {
    emit accountDialogClosed( false );
  }
}

void AccountsController::deleteAccount()
{
  if (!model)
    return;

  if ( !view->selectionModel()->currentIndex().isValid() )
    return;

  int row = view->selectionModel()->currentIndex().row();
  model->removeRow( row );
  settings.deleteAccount( row, model->rowCount() + 1 );
  if ( model->rowCount() <= 0 ) {
    ui->deleteAccountButton->setEnabled( false );
  } else {
    ui->deleteAccountButton->setEnabled( true );
  }
}

void AccountsController::setAccountEnabled( bool state )
{
  if (!model)
    return;

  if ( !view->selectionModel()->currentIndex().isValid() )
    return;

  model->account( view->currentIndex().row() ).isEnabled = state;
  settings.setValue( QString("Accounts/%1/enabled").arg( view->currentIndex().row() ), state );
}

void AccountsController::setAccountDM( bool state )
{
  if (!model)
    return;

  if ( !view->selectionModel()->currentIndex().isValid() )
    return;

  model->account( view->currentIndex().row() ).directMessages = state;
  settings.setValue( QString("Accounts/%1/directmsgs").arg( view->currentIndex().row() ), state );
}

void AccountsController::retranslateUi()
{
  if ( widget ) {
    ui->retranslateUi( widget );
  }
}
