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
#include <qtwitterapp.h>
#include <qticonloader.h>

#include <oauthwizard.h>

#include <QMessageBox>
#include <QInputDialog>
#include <QModelIndex>
#include <QDebug>

extern ConfigFile settings;

AccountsController::AccountsController( QWidget *widget, QObject *parent ) :
        QObject( parent ),
        model( 0 ),
        ui( new Ui::Accounts ),
        modified( false ),
        widget( widget )
{
    ui->setupUi( widget );
    view = ui->accountsView;

    //> freedesktop experiment begin
    ui->addAccountButton->setIcon(QtIconLoader::icon("list-add", QIcon(":/icons/add_48.png")));
    ui->deleteAccountButton->setIcon(QtIconLoader::icon("list-remove", QIcon(":/icons/cancel_48.png")));
    //< freedesktop experiment end

    bool savePasswords = true;
    if ( settings.contains( "General/savePasswords" ) ) {
        savePasswords = settings.value( "General/savePasswords", true ).toBool();
    } else {
        settings.setValue( "General/savePasswords", savePasswords );
        settings.sync();
    }
    ui->passwordsCheckBox->setChecked( savePasswords );

    connect( view, SIGNAL(checkBoxClicked(QModelIndex)), this, SLOT(updateCheckBox(QModelIndex)) );
    connect( ui->addAccountButton, SIGNAL(clicked()), this, SLOT(addAccount()));
    connect( ui->deleteAccountButton, SIGNAL(clicked()), this, SLOT(deleteAccount()));
    connect( ui->passwordsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(togglePasswordStoring(int)) );
    connect( ui->disclaimerButton, SIGNAL(clicked()), this, SLOT(showPasswordDisclaimer()) );

    view->setItemDelegate( new AccountsDelegate( this ) );

    Account::setNetworkName( Account::NetworkUrlTwitter, Account::NetworkTwitter );
    Account::setNetworkName( Account::NetworkUrlIdentica, Account::NetworkIdentica );
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

bool AccountsController::isModified() const
{
    return modified;
}

void AccountsController::setModified( bool modified )
{
    this->modified = modified;
}

void AccountsController::loadCustomNetworks()
{
    settings.beginGroup( "Services" );
    QList<QString> keys = settings.childKeys();
    settings.endGroup();
    foreach( QString key, keys ) {
        Account::setNetworkName( settings.value( QString("Services/%1").arg(key),
                                                 "other/unknown" ).toString(), key );
    }
}

void AccountsController::loadAccounts()
{
    if (!model)
        return;

    settings.beginGroup( "Accounts" );
    int accountsCount = settings.childGroups().count();

    QList<Account> modelAccounts = model->getAccounts();
    QList<Account> settingsAccounts;
    for ( int i = 0; i < accountsCount; i++ ) {
        Account account;
        QString id = QString::number(i);
        account.setEnabled( settings.value( QString( "%1/enabled" ).arg(id), false ).toBool() );
        account.setServiceUrl( settings.value( QString( "%1/service" ).arg(id), Account::NetworkUrlTwitter ).toString() );
        account.setLogin( settings.value( QString( "%1/login" ).arg(id), "" ).toString() );
        account.setDM( settings.value( QString( "%1/directmsgs" ).arg(id), false ).toBool() );

        account.setPassword( settings.pwHash( settings.value( QString( "%1/password" ).arg(id), "" ).toString() ) );
        settingsAccounts << account;
        //    passwords << account.password();
    }
    settings.endGroup(); //Accounts

    for( int i = 0; i < modelAccounts.size(); ++i ) {
        Account &modelAccount = modelAccounts[i];
        if ( !settingsAccounts.contains( modelAccount ) ) {
            modelAccounts.removeAll( modelAccount );
        }
    }

    for( int i = 0; i < settingsAccounts.size(); ++i ) {
        Account &settingsAccount = settingsAccounts[i];
        if ( !modelAccounts.contains( settingsAccount ) ) {
            modelAccounts.insert( i, settingsAccount );
        } else {
            int j = modelAccounts.indexOf( settingsAccount );
            if ( j != -1 && i < modelAccounts.size() - 1 ) {
                modelAccounts.move(j, i);
            }
        }
    }

    qSort( modelAccounts );
    model->setAccounts( modelAccounts );
    updateAccounts( model->index(0,0), model->index( model->rowCount() - 1, model->columnCount() - 1 ) );
    int size = settingsAccounts.size();
    if ( size > modelAccounts.size() ) {
        for ( int k = modelAccounts.size(); k < size; ++k ) {
            settings.deleteAccount( k, size );
            size--;
        }
    }

    if ( view->model()->rowCount() <= 0 ) {
        ui->deleteAccountButton->setEnabled( false );
    } else {
        ui->deleteAccountButton->setEnabled( true );
    }

    modified = true;
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
                if ( settings.value( "General/savePasswords", true ).toBool() ||
                     model->index(i, AccountsModel::COL_NETWORK ).data( Qt::EditRole ) == Account::NetworkUrlTwitter ) {
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
    settings.sync();
    modified = true;
}

void AccountsController::updateCheckBox( const QModelIndex &index )
{
    if (!model)
        return;

    Account &account = model->account( index.row() );
    if ( index.column() == AccountsModel::COL_ENABLED ) {
        account.setEnabled( !account.isEnabled() );
        setAccountEnabled( account.isEnabled() );
    } else if ( index.column() == AccountsModel::COL_DM ) {
        account.setDM( !account.dm() );
        setAccountDM( account.dm() );
    }
    view->update( index );
    modified = true;
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
            if ( model->account(i).serviceUrl() == Account::NetworkUrlIdentica ) {
                settings.remove( QString( "Accounts/%1/password" ).arg(i) );
            }
        }
    }
    bool checked = ( state == Qt::Checked ) ? true : false;
    settings.setValue( "General/savePasswords", checked );
    settings.sync();
}

void AccountsController::showPasswordDisclaimer()
{
    QMessageBox messageBox( QMessageBox::Warning, tr( "Password security" ), tr( "Please note:" ), QMessageBox::Ok );
    messageBox.setInformativeText( tr( "Although passwords are stored as human unreadable data, "
                                       "they can be easily decoded using the application's source code, "
                                       "which is publicly available. You have been warned." ) );
    messageBox.setInformativeText( messageBox.informativeText().append( "<br><br>" )
                                   .append( tr( "Note also that Twitter authorization keys are stored anyway. "
                                                "Remove the account from the list if you want the key to be deleted."
                                                /*"They can't be reused outside this application."*/ ) ) );
    messageBox.exec();
}

void AccountsController::addAccount()
{
    if (!model)
        return;

    int result;
    QString networkName;
    QString serviceUrl;
    QString login;
    QString password;

    QWidget *parent = 0;
    if ( sender() ) {
        parent = qobject_cast<QWidget*>( sender() );
    }

    NewAccountDialog *dlg = new NewAccountDialog( parent ? parent : QTwitterApp::instance()->activeWindow() );
    result = dlg->exec();
    networkName = dlg->networkName();
    serviceUrl = dlg->serviceUrl();
    login = dlg->login();
    password = dlg->password();
    delete dlg;

    if ( result == QDialog::Accepted ) {

        if ( networkName != Account::NetworkTwitter &&
             networkName != Account::NetworkIdentica ) {
            Account::setNetworkName( serviceUrl, networkName );
        }
        int index = model->rowCount();

        if ( networkName == Account::NetworkTwitter ) {
            OAuthWizard *wizard = new OAuthWizard( view );
            wizard->exec();
            if ( wizard->authorized() ) {
                model->insertRow( index );

                Account &account = model->account( index );
                account.setServiceUrl( Account::NetworkUrlTwitter );
                account.setLogin( wizard->getScreenName() );
                account.setPassword( wizard->getOAuthKey() );
                settings.addAccount( index, account );
                view->setCurrentIndex( model->index( index, 0 ) );
                ui->deleteAccountButton->setEnabled( true );
                emit accountDialogClosed( true );
            } else {
                emit accountDialogClosed( false );
            }
            wizard->deleteLater();

        } else {
            model->insertRow( index );
            Account &account = model->account( index );
            account.setServiceUrl( Account::networkUrl( networkName ) );
            account.setLogin( login );
            account.setPassword( password );
            settings.addAccount( index, account );
            view->setCurrentIndex( model->index( index, 0 ) );
            ui->deleteAccountButton->setEnabled( true );
            emit accountDialogClosed( true );
        }
    } else {
        emit accountDialogClosed( false );
    }
    modified = true;
}

void AccountsController::deleteAccount()
{
    if (!model)
        return;

    if ( !view->selectionModel()->currentIndex().isValid() )
        return;

    int row = view->selectionModel()->currentIndex().row();
    QString networkName = Account::networkName( model->account( row ).serviceUrl() );
    model->removeRow( row );
    settings.deleteAccount( row, model->rowCount() + 1 );
    if ( model->rowCount() <= 0 ) {
        ui->deleteAccountButton->setEnabled( false );
    } else {
        ui->deleteAccountButton->setEnabled( true );
    }
    if ( networkName != Account::NetworkIdentica &&
         networkName != Account::NetworkTwitter ) {
        bool exists = false;
        foreach ( Account account, model->getAccounts() ) {
            if ( networkName == Account::networkName( account.serviceUrl() ) ) {
                exists = true;
                break;
            }
        }
        if ( !exists ) {
            Account::removeNetwork( Account::networkUrl( networkName ) );
        }
    }
}

void AccountsController::setAccountEnabled( bool state )
{
    if (!model)
        return;

    if ( !view->selectionModel()->currentIndex().isValid() )
        return;

    model->account( view->currentIndex().row() ).setEnabled( state );
    settings.setValue( QString("Accounts/%1/enabled").arg( view->currentIndex().row() ), state );
}

void AccountsController::setAccountDM( bool state )
{
    if (!model)
        return;

    if ( !view->selectionModel()->currentIndex().isValid() )
        return;

    model->account( view->currentIndex().row() ).setDM( state );
    settings.setValue( QString("Accounts/%1/directmsgs").arg( view->currentIndex().row() ), state );
    modified = true;
}

void AccountsController::retranslateUi()
{
    if ( widget ) {
        ui->retranslateUi( widget );
    }
}
