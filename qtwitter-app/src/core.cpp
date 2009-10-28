/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#include <QDesktopServices>
#include <QProcess>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>
#include <QDebug>
#include <QDataStream>
#include <QStringList>
#include <QDir>
#include <QFile>
#include "core.h"
#include "mainwindow.h"
#include "imagedownload.h"
#include "dmdialog.h"
#include "configfile.h"
#include "qtwitterapp.h"
#include "twitpicengine.h"
#include "statusmodel.h"
#include "statuswidget.h"
#include "welcomedialog.h"
#include "accountsmodel.h"
#include "accountscontroller.h"
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"

#include <oauthwizard.h>

extern ConfigFile settings;

Core::Core( QObject *parent ) :
        QObject( parent ),
        authDialogOpen( false ),
        m_requestCount(0),
        m_checkForUnread(CheckForUnread),
        waitForAccounts( false ),
        settingsOpen( false ),
        twitpicUpload(0),
        accounts(0),
        accountsModel(0),
        timer(0)
{
    connect( ImageDownload::instance(), SIGNAL(imageReadyForUrl(QString,QPixmap*)), this, SLOT(setImageForUrl(QString,QPixmap*)) );

    connect( StatusModel::instance(), SIGNAL(openBrowser(QUrl)), this, SLOT(openBrowser(QUrl)) );
    connect( StatusModel::instance(), SIGNAL(markEverythingAsRead()), this, SLOT(markEverythingAsRead()) );

    accountsModel = new AccountsModel;
}

void Core::storeSession()
{
#if defined Q_WS_MAC || defined Q_WS_WIN
    QString path( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/qTwitter" );
#else
    QString path( QDesktopServices::storageLocation( QDesktopServices::HomeLocation ) + "/.qtwitter" );
#endif
    QDir dir( path );
    if ( !dir.exists() && !dir.mkpath( path ) ) {
        return;
    }
    QFile file( path + "/state" );
    bool ok = file.open(QIODevice::WriteOnly);
    if (ok) {
        QDataStream out(&file);
        out << ConfigFile::APP_VERSION;
        QList<Account*> accountsList = statusLists.keys();
        StatusList *list = 0;
        out << accountsList.size();
        foreach ( Account *account, accountsList ) {
            out << *account;
            list = statusLists.value( account );
            out << (qint8) list->isVisible();
            out << (qint8) list->active();
            out << list->getData();
        }
        file.close();
    }
    settings.setValue( "cleanexit", true );
    settings.sync();
}

void Core::restoreSession()
{
#if defined Q_WS_MAC || defined Q_WS_WIN
    QFile file( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/qTwitter/state" );
#else
    QFile file( QDesktopServices::storageLocation( QDesktopServices::HomeLocation ) + "/.qtwitter/state" );
#endif

    if ( settings.value( "cleanexit", true ).toBool() == false ) {
        QMessageBox *mbox = new QMessageBox;

        mbox->setWindowTitle( tr( "Warning" ) );
        mbox->setText( tr( "It seems like qTwitter wasn't closed properly last time. "
                           "If the application crashed, it may be due to "
                           "inconsistency in accounts settings. Do you want "
                           "to reset accounts in qTwitter?" ) );
        mbox->setInformativeText( tr( "If you think that it's a bug, please report it "
                                      "<a href=\"http://ayoy.lighthouseapp.com/projects/"
                                      "27230-qtwitter/tickets?q=all\">here</a>" ) );
        mbox->setIconPixmap( QPixmap( ":/icons/twitter_48.png" ) );

        QAbstractButton *yes = mbox->addButton( tr( "&Yes, reset accounts settings" ), QMessageBox::AcceptRole );
        mbox->addButton( tr( "&No, thanks" ), QMessageBox::RejectRole );

        mbox->exec();
        QAbstractButton *clicked = mbox->clickedButton();
        delete mbox;

        if ( clicked == yes ) {
            if ( file.exists() ) {
                file.remove();
            }
            settings.beginGroup( "Accounts" );
            settings.remove("");
            settings.endGroup();
            settings.sync();
        }
    }

    bool ok = file.open(QIODevice::ReadOnly);
    if (ok) {
        QDataStream in(&file);
        QString version;
        in >> version;
        if ( version != ConfigFile::APP_VERSION ) {
            file.close();
            return;
        }
        int count;
        in >> count;
        //    qDebug() << "Stream status:" << in.status();
        if ( in.status() == QDataStream::Ok ) {
            //      qDebug() << __FUNCTION__ << "Accounts count:" << count;

            QList<Account> accountsList;
            StatusList* statusList = 0;

            for( int i = 0; i < count; ++i ) {
                Account *account = new Account;
                in >> *account;
                accountsList << *account;
                statusList = new StatusList( account, this );
                qint8 visible;
                qint8 active;
                QList<Status> list;
                in >> visible;
                in >> active;
                in >> list;
                statusList->setVisible( (bool) visible );
                statusList->setActive( active );
                statusList->setStatuses( list );
                statusLists.insert( account, statusList );
            }
        }
    }
    file.close();

    settings.setValue( "cleanexit", false );
    settings.sync();
}

void Core::createAccounts( QWidget *view )
{
    accounts = new AccountsController( view, this );
    connect( accounts, SIGNAL(comboActive(bool)), this, SLOT(setWaitForAccounts(bool)) );
    connect( accounts, SIGNAL(accountDialogClosed(bool)), this, SIGNAL(accountDialogClosed(bool)) );
    if ( accountsModel )
        accounts->setModel( accountsModel );
    else
        qWarning() << "Accounts model not present!";
}

void Core::setWaitForAccounts( bool wait )
{
    if ( !settingsOpen && waitForAccounts && !wait ) {
        waitForAccounts = wait;
        applySettings();
        if ( accounts->isModified() ) {
            get();
        }
        return;
    }
    waitForAccounts = wait;
}

void Core::markEverythingAsRead()
{
    foreach ( StatusList *statusList, statusLists.values() ) {
        statusList->markAllAsRead();
    }
}

void Core::setSettingsOpen( bool open )
{
    settingsOpen = open;
    if ( open ) {
        accounts->setModified( false );
    }
}

QStringList Core::twitpicLogins() const
{
    QStringList list;
    foreach ( Account account, accountsModel->getAccounts() ) {
        if ( account.serviceUrl() == Account::NetworkUrlTwitter ) {
            list << account.login();
        }
    }
    return list;
}

void Core::applySettings()
{
    // TODO: Execute on startup only
    if ( settings.contains( "FIRSTRUN" ) ) {
        WelcomeDialog dlg;
        connect( &dlg, SIGNAL(addAccount()), this, SLOT(addAccount()) );
        connect( this, SIGNAL(accountDialogClosed(bool)), &dlg, SLOT(confirmAccountAdded(bool)) );
        dlg.exec();
        settings.remove( "FIRSTRUN" );
    }

    static bool appStartup = true;
    if ( appStartup ) {
        accounts->loadCustomNetworks();
        accounts->loadAccounts();
        appStartup = false;
    }

    if ( !waitForAccounts ) {

        int mtc = settings.value( "Appearance/tweet count", 20 ).toInt();
        StatusList::setMaxCount( mtc );
        StatusModel::instance()->setMaxStatusCount( mtc );
        setTimerInterval( settings.value( "General/refresh-value", 15 ).toInt() * 60000 );

        setupStatusLists();
        if ( statusLists.count() == 0 ) {
            StatusModel::instance()->clear();
        }

        emit accountsUpdated( accountsModel->getAccounts() );

        emit resetUi();
        m_requestCount = 0;

        if ( accounts->isModified() ) {
            get();
        }
    }
}

bool Core::setTimerInterval( int msecs )
{
    bool initialization = !(bool) timer;
    if ( initialization ) {
        timer = new QTimer( this );
        connect( timer, SIGNAL(timeout()), this, SLOT(get()) );
    }
    if ( timer->interval() != msecs ) {
        timer->setInterval( msecs );
        timer->start();
        if ( !initialization ) {
            return true;
        }
    }
    return false;
}

#ifdef Q_WS_X11
void Core::setBrowserPath( const QString &path )
{
    browserPath = path;
}
#endif

void Core::setModelTheme( const ThemeData &theme )
{
    StatusModel::instance()->setTheme( theme );
}

void Core::setModelData( const QString &serviceUrl, const QString &login )
{
    //TODO: debug, warning, etc.
    if ( login.isNull() ) {
        StatusModel::instance()->clear();
    } else {
        Account *accountToShow = accountsModel->account( serviceUrl, login );
        foreach( Account *account, statusLists.keys() ) {
            if ( *account == *accountToShow ) {
                StatusModel::instance()->setStatusList( statusLists[ account ] );
                break;
            }
        }
    }
}

void Core::forceGet()
{
    timer->start();
    get();
}

void Core::get( const QString &serviceUrl, const QString &login, const QString &password )
{
    // TODO: remove
    Q_UNUSED(password);

    Account *account = accountsModel->account( serviceUrl, login );
    if ( account ) {
        StatusList *statusList = statusLists.value( account );
        if ( statusList ) {
            statusList->requestFriendsTimeline();
            statusList->requestMentions();
            if ( statusList->dm() ) {
                statusList->requestDirectMessages();
            }
        }
    }
}

void Core::get()
{
    bool started = false;
    foreach( StatusList *statusList, statusLists.values() ) {
        started = true;
        statusList->requestFriendsTimeline();
        statusList->requestMentions();
        if ( statusList->dm() ) {
            statusList->requestDirectMessages();
        }
    }

    if ( started ) {
        emit requestStarted();
    }
}

Account* Core::findAccount( const Account &account )
{
    Account *ret = 0;

    foreach ( Account *a, statusLists.keys() ) {
        if ( *a == account ) {
            ret = a;
            break;
        }
    }
    return ret;
}

void Core::post( const QString &serviceUrl, const QString &login, const QString &status, quint64 inReplyTo )
{
    Account *a = accountsModel->account( serviceUrl, login );
    if ( a ) {
        Account *account = findAccount( *a );
        if ( account ) {
            StatusList *statusList = statusLists.value( account );
            if ( statusList ) {
                statusList->requestNewStatus( status, inReplyTo );
            }
        }
    }
    emit requestStarted();
}

void Core::uploadPhoto( const QString &login, QString photoPath, QString status )
{
    bool ok = false;
    QString password = QInputDialog::getText( 0, tr( "Enter password" ), tr( "Enter your Twitter password.<br>We're not storing it anywhere" ),
                                              QLineEdit::Password, QString(), &ok );
    if ( ok && !password.isEmpty() ) {
        twitpicUpload = new TwitPicEngine( this );
        qDebug() << "uploading photo";
        twitpicUpload->postContent( login, password, photoPath, status );
    }
}

void Core::abortUploadPhoto()
{
    if ( twitpicUpload ) {
        twitpicUpload->abort();
        twitpicUpload->deleteLater();
        twitpicUpload = 0;
    }
}

void Core::twitPicResponse( bool responseStatus, QString message, bool newStatus )
{
    qDebug() << "twicPicResponse";
    emit twitPicResponseReceived();
    if ( !responseStatus ) {
        emit errorMessage( tr( "There was a problem uploading your photo:" ).append( " %1" ).arg( message ) );
        return;
    }
    if ( newStatus ) {
        forceGet();
    }
    twitpicUpload->deleteLater();
    twitpicUpload = 0;
    QDialog dlg;
    Ui::TwitPicNewPhoto ui;
    ui.setupUi( &dlg );
    ui.textBrowser->setText( tr( "Photo available at:" ).append( " <a href=\"%1\">%1</a>" ).arg( message ) );
    dlg.exec();
}

void Core::openBrowser( QUrl address )
{
    if ( address.isEmpty() )
        return;
#if defined Q_WS_MAC || defined Q_WS_WIN
    QDesktopServices::openUrl( address );
#elif defined Q_WS_X11
    QProcess *browser = new QProcess;
    if ( browserPath.isNull() ) {
        QDesktopServices::openUrl( address );
        return;
    }
    browser->start( browserPath + " " + address.toString() );
#endif
}

Core::AuthDialogState Core::authDataDialog( Account *account )
{
    if ( authDialogOpen )
        return Core::STATE_DIALOG_OPEN;
    emit pauseIcon();
    QDialog *dlg = new QDialog( QTwitterApp::instance()->activeWindow() );
    Ui::AuthDialog ui;
    ui.setupUi( dlg );
    QString ident = QString( "<br>%1 @%2<br>" ).arg( account->login(),
                                                     Account::networkName( account->serviceUrl() ) );
    ui.titleLabel->setText( ui.titleLabel->text().arg( ident ) );
    //: This is for newly created account - when the login isn't given yet
    ui.loginEdit->setText( ( account->login() == tr( "<empty>" ) ) ? QString() : account->login() );
    ui.loginEdit->selectAll();
    ui.passwordEdit->setText( account->password() );
    dlg->adjustSize();
    authDialogOpen = true;
    int row = accountsModel->indexOf( *account );
    if ( dlg->exec() == QDialog::Accepted ) {
        if ( ui.disableBox->isChecked() ) {
            authDialogOpen = false;
            account->setEnabled( false );
            settings.setValue( QString("Accounts/%1/enabled").arg( row ), false );
            emit accountsUpdated( accountsModel->getAccounts() );
            delete dlg;
            return Core::STATE_DISABLE_ACCOUNT;
        } else if ( ui.removeBox->isChecked() ) {
            authDialogOpen = false;
            accountsModel->removeRow( row );
            settings.deleteAccount( row, accountsModel->rowCount() );
            emit accountsUpdated( accountsModel->getAccounts() );
            delete dlg;
            return Core::STATE_REMOVE_ACCOUNT;
        }
        account->setLogin( ui.loginEdit->text() );
        account->setPassword( ui.passwordEdit->text() );
        accountsModel->account( row ).setLogin( ui.loginEdit->text() );
        accountsModel->account( row ).setPassword( ui.passwordEdit->text() );

        settings.setValue( QString("Accounts/%1/login").arg( row ), account->login() );

        if ( settings.value( "General/savePasswords", false ).toBool() )
            settings.setValue( QString("Accounts/%1/password").arg( row ), ConfigFile::pwHash( account->password() ) );
        settings.sync();

        authDialogOpen = false;
        emit requestStarted();
        delete dlg;
        return Core::STATE_ACCEPTED;
    }
    authDialogOpen = false;
    delete dlg;
    return Core::STATE_REJECTED;
}

void Core::retranslateUi()
{
    StatusModel::instance()->retranslateUi();
    if ( accounts )
        accounts->retranslateUi();
}


void Core::setImageForUrl( const QString& url, QPixmap *image )
{
    Status status;
    foreach ( StatusList *statusList, statusLists )
    {
        for ( int i = 0; i < statusList->size(); i++ ) {
            status = statusList->data(i);
            if ( status.entry.type == Entry::Status && url == status.entry.userInfo.imageUrl ) {
                statusList->setImage( i, *image );
            }
        }
    }
}

void Core::setupStatusLists()
{
    accountsModel->cleanUp();
    QList<Account> modelAccounts = accountsModel->getAccounts(); // get accounts from model

    Account dmAccount;
    // delete status lists for accounts removed from model
    foreach( Account *account, statusLists.keys() ) {
        dmAccount = *account;
        dmAccount.setDM( !account->dm() );
        if ( !modelAccounts.contains( *account ) ) {
            if ( modelAccounts.contains( dmAccount ) ) {
                account->setDM( !account->dm() );
            } else {
                statusLists[ account ]->deleteLater();
                delete account;
                qDebug() << "Deleting statusList for account:" << account->login() << account->serviceUrl();
                statusLists.remove( account );
            }
        }
    }

    // check modelAccounts one by one
    foreach ( Account modelAccount, modelAccounts ) {
        if ( modelAccount.isEnabled() ) {    // if it's enabled
            bool contains = false;             // check if a status list for it exists
            foreach( Account *account, statusLists.keys() ) {
                if ( (*account).fuzzyCompare( modelAccount ) ) {
                    contains = true;
                    break;
                }
            }
            if ( !contains ) {     // if the status list doesn't exists
                Account *newAccount = new Account( modelAccount ); // create one
                StatusList *newStatusList = new StatusList( newAccount, this );
                statusLists.insert( newAccount, newStatusList ); // and add it
            }
        } else {   // if account is not enabled
            Account *accountToRemove = 0; // check if a status list for it exists, and store result in a pointer
            foreach( Account *account, statusLists.keys() ) {
                if ( *account == modelAccount ) {
                    accountToRemove = account;
                    break;
                }
            }
            if ( accountToRemove ) {  // if a status list exist for a removed account
                statusLists[ accountToRemove ]->deleteLater(); // remove it
                statusLists.remove( accountToRemove );
                delete accountToRemove; // and delete the account
            }
        }
    }

    foreach ( Account *account, statusLists.keys() ) {
        statusLists[ account ]->slotDirectMessagesChanged( account->dm() );
    }

    m_requestCount = 0;
}

bool Core::retryAuthorizing( Account *account, int role )
{
    if ( !account )
        return false;

    Core::AuthDialogState state = authDataDialog( account );
    switch ( state ) {
    case Core::STATE_ACCEPTED:
        return true;
    case Core::STATE_REJECTED:
        switch ( role ) {
        case TwitterAPI::ROLE_POST_UPDATE:
            emit errorMessage( tr( "Authentication is required to post updates." ) );
            break;
        case TwitterAPI::ROLE_POST_DM:
            emit errorMessage( tr( "Authentication is required to send direct messages." ) );
            break;
        case TwitterAPI::ROLE_DELETE_UPDATE:
            emit errorMessage( tr( "Authentication is required to delete updates." ) );
            break;
        case TwitterAPI::ROLE_FRIENDS_TIMELINE:
            emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
            break;
        case TwitterAPI::ROLE_DIRECT_MESSAGES:
            break;
        case TwitterAPI::ROLE_PUBLIC_TIMELINE:
            break;
        }
    case Core::STATE_DIALOG_OPEN:
    case Core::STATE_REMOVE_ACCOUNT:
    case Core::STATE_DISABLE_ACCOUNT:
    default:;
    }
    return false;
}

void Core::resetRequestsCount()
{
    if ( m_requestCount > 0 ) {
        m_requestCount = 0;
        qDebug() << "warning: some requests may have failed...";
    }
}

void Core::addAccount()
{
    accounts->addAccount();
}

void Core::slotRequestDone( const QString &serviceUrl, const QString &login, int role )
{
    StatusList *statusList = StatusModel::instance()->getStatusList();
    if ( statusList->serviceUrl() == serviceUrl
         && statusList->login() == login ){
        StatusModel::instance()->updateDisplay();
    }
    if ( role != TwitterAPI::ROLE_POST_DM && m_requestCount > 0 ) {
        m_requestCount--;
    }
    qDebug() << m_requestCount;
    if ( m_requestCount == 0 ) {
        if ( m_checkForUnread == CheckForUnread )
            checkUnreadStatuses();
        emit resetUi();
    }
    m_checkForUnread = CheckForUnread;
}

void Core::checkUnreadStatuses()
{
    QStringList unread;
    QString message;
    foreach ( Account *account, statusLists.keys() ) {
        int newStatuses = statusLists[ account ]->newStatusesCount();
        if ( newStatuses > 0 ) {
            unread.append( QString( "%1 @%2 (%3)" ).arg( account->login(),
                                                         Account::networkName( account->serviceUrl() ),
                                                         QString::number(newStatuses) ) );
        }
    }

    if ( unread.isEmpty() )
        return;

    if ( unread.count() == 1 ) {
        message.append( unread.at(0) );
        //: "For <user_name>"
        emit sendNewsReport( tr( "For %1" ).arg(message) );
    } else {
        message.append( unread.join( "\n" ) );
        //: There goes "For", a colon, a new line, and a list of users that heave unread statuses.
        emit sendNewsReport( tr( "For:\n%1" ).arg(message) );
    }
}
