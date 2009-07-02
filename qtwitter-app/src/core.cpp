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
#include <QInputDialog>
#include <QDebug>
#include <QDataStream>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <urlshortener/urlshortener.h>
#include "core.h"
#include "mainwindow.h"
#include "imagedownload.h"
#include "dmdialog.h"
#include "configfile.h"
#include "twitpicengine.h"
#include "statusmodel.h"
#include "statuswidget.h"
#include "welcomedialog.h"
#include "accountsmodel.h"
#include "accountscontroller.h"
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"

#ifdef OAUTH
#  include <oauthwizard.h>
#endif

extern ConfigFile settings;

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    authDialogOpen( false ),
    requestCount(0),
    tempModelCount(0),
    waitForAccounts( false ),
    settingsOpen( false ),
    checkForNew( true ),
    twitpicUpload(0),
    accounts(0),
    accountsModel(0),
    timer(0),
    parentMainWindow( parent )
{
  imageDownload = new ImageDownload( this );
  connect( imageDownload, SIGNAL(imageReadyForUrl(QString,QPixmap*)), this, SLOT(setImageForUrl(QString,QPixmap*)) );

  twitterapi = new TwitterAPIInterface( this );

#ifdef OAUTH
  twitterapi->setConsumerKey( OAuthWizard::ConsumerKey );
  twitterapi->setConsumerSecret( OAuthWizard::ConsumerSecret );
#endif

  connect( twitterapi, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)), this, SLOT(addEntry(TwitterAPI::SocialNetwork,QString,Entry)) );
  connect( twitterapi, SIGNAL(deleteEntry(TwitterAPI::SocialNetwork,QString,quint64)), this, SLOT(deleteEntry(TwitterAPI::SocialNetwork,QString,quint64)) );
  connect( twitterapi, SIGNAL(favoriteStatus(TwitterAPI::SocialNetwork,QString,quint64,bool)), this, SLOT(setFavorited(TwitterAPI::SocialNetwork,QString,quint64,bool)) );
  connect( twitterapi, SIGNAL(postDMDone(TwitterAPI::SocialNetwork,QString,TwitterAPI::ErrorCode)), this, SIGNAL(confirmDMSent(TwitterAPI::SocialNetwork,QString,TwitterAPI::ErrorCode)) );
  connect( twitterapi, SIGNAL(deleteDMDone(TwitterAPI::SocialNetwork,QString,quint64,TwitterAPI::ErrorCode)), this, SLOT(deleteEntry(TwitterAPI::SocialNetwork,QString,quint64)) );
  connect( twitterapi, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString,QString,quint64)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString,QString,quint64)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString,quint64,Entry::Type)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString,quint64,Entry::Type)) );

  connect( this, SIGNAL(newRequest()), SLOT(slotNewRequest()) );
  connect( twitterapi, SIGNAL(requestDone(TwitterAPI::SocialNetwork,QString,int)), this, SLOT(slotRequestDone(TwitterAPI::SocialNetwork,QString,int)) );

  listViewForModels = parent->getListView();

  urlShortener = new UrlShortener( this );
  connect( urlShortener, SIGNAL(shortened(QString)), this, SIGNAL(urlShortened(QString)));
  connect( urlShortener, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

  statusModel = new StatusModel( listViewForModels, this );
  parent->setListViewModel( statusModel );

  connect( statusModel, SIGNAL(openBrowser(QUrl)), this, SLOT(openBrowser(QUrl)) );
  connect( statusModel, SIGNAL(reply(QString,quint64)), this, SIGNAL(addReplyString(QString,quint64)) );
  connect( statusModel, SIGNAL(about()), this, SIGNAL(about()) );
  connect( statusModel, SIGNAL(destroy(TwitterAPI::SocialNetwork,QString,quint64,Entry::Type)), this, SLOT(destroy(TwitterAPI::SocialNetwork,QString,quint64,Entry::Type)) );
  connect( statusModel, SIGNAL(favorite(TwitterAPI::SocialNetwork,QString,quint64,bool)), this, SLOT(favoriteRequest(TwitterAPI::SocialNetwork,QString,quint64,bool)) );
  connect( statusModel, SIGNAL(postDM(TwitterAPI::SocialNetwork,QString,QString)), this, SLOT(postDMDialog(TwitterAPI::SocialNetwork,QString,QString)) );
  connect( statusModel, SIGNAL(retweet(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( statusModel, SIGNAL(markEverythingAsRead()), this, SLOT(markEverythingAsRead()) );
  connect( this, SIGNAL(resizeData(int,int)), statusModel, SLOT(resizeData(int,int)) );

  accountsModel = new AccountsModel;

#if defined Q_WS_MAC || Q_WS_WIN
  QFile file( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/qTwitter/state" );
#else
  QFile file( QDesktopServices::storageLocation( QDesktopServices::HomeLocation ) + "/.qtwitter/state" );
#endif
  bool ok = file.open(QIODevice::ReadOnly);
  if (ok) {
    QDataStream in(&file);
    int count;
    in >> count;
//    qDebug() << "Stream status:" << in.status();
    if ( in.status() == QDataStream::Ok ) {
//      qDebug() << __FUNCTION__ << "Accounts count:" << count;

      QList<Account> accountsList;

      for( int i = 0; i < count; ++i ) {
        Account account;
        in >> account;
        accountsList << account;
        statusLists.insert( account, new StatusList( account.login, account.network, this ) );
      }

//      qDebug() << "Stream status:" << in.status();
      if ( in.status() == QDataStream::Ok ) {
        accountsModel->setAccounts( accountsList );

        foreach( StatusList *statusList, statusLists.values() ) {
          qint8 visible;
          qint8 active;
          QList<Status> list;
          in >> visible;
          in >> active;
          in >> list;
          statusList->setStatuses( list );
          statusList->setVisible( (bool) visible );
          statusList->setActive( active );
        }
      }
//      qDebug() << "Stream status:" << in.status();
    }
  }
  file.close();
}

Core::~Core()
{
#if defined Q_WS_MAC || Q_WS_WIN
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
    QList<Account> list = statusLists.keys();
    out << list.size();
    foreach ( Account account, list ) {
      out << account;
    }
    foreach ( StatusList *statusList, statusLists.values() ) {
      out << (qint8) statusList->isVisible();
      out << (qint8) statusList->active();
      out << statusList->getData();
    }
    file.close();
  }
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
    get();
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
}

QStringList Core::twitpicLogins() const
{
  QStringList list;
  foreach ( Account account, accountsModel->getAccounts() ) {
    if ( account.network == TwitterAPI::SOCIALNETWORK_TWITTER ) {
      list << account.login;
    }
  }
  return list;
}

void Core::applySettings()
{
  if ( settings.contains( "FIRSTRUN" ) || settings.value( "OAuth", true ) == false ) {
    QString val = settings.value( "FIRSTRUN" ).toString();
    if ( val == "ever" ) {
      WelcomeDialog dlg;
      connect( &dlg, SIGNAL(addAccount()), this, SLOT(addAccount()) );
      connect( this, SIGNAL(accountDialogClosed(bool)), &dlg, SLOT(confirmAccountAdded(bool)) );
      dlg.exec();
    }
#ifdef OAUTH
    else if ( val == ConfigFile::APP_VERSION || settings.value( "OAuth", true ) == false ) {
      settings.removeOldTwitterAccounts();
      QMessageBox dlg;
      dlg.setWindowTitle( tr( "Welcome!" ) );
      //: e.g. "Welcome to qTwitter 0.8.0!"
      dlg.setText( tr( "Welcome to qTwitter %1!" ).arg( ConfigFile::APP_VERSION ) );
      dlg.setInformativeText( tr( "<b>Please note:</b> we introduced the ultra-secure OAuth "
                                  "authorization scheme for Twitter accounts in this release. This "
                                  "means that you would never be asked again to enter your Twitter "
                                  "password in qTwitter. However, your existing qTwitter "
                                  "configuration will not work any more. Your Twitter accounts and "
                                  "passwords have been removed from qTwitter settings, please "
                                  "reconfigure them in Settings to have things working. "
                                  "Thanks and enjoy qTwitter!" ) );
      dlg.setIconPixmap( QPixmap( ":icons/twitter_48.png" ) );
      dlg.exec();
      settings.setValue( "OAuth", true );
    }
#endif
    settings.remove( "FIRSTRUN" );
  }

  static bool appStartup = true;
  if ( appStartup ) {
    accounts->loadAccounts();
    appStartup = false;
  }

  if ( !waitForAccounts ) {

    int mtc = settings.value( "Appearance/tweet count", 20 ).toInt();
    StatusList::setMaxCount( mtc );
    statusModel->setMaxStatusCount( mtc );
    setTimerInterval( settings.value( "General/refresh-value", 15 ).toInt() * 60000 );

    setupStatusLists();
    if ( statusLists.count() == 0 ) {
      statusModel->clear();
    }

    emit accountsUpdated( accountsModel->getAccounts() );

    // TODO: do this only when really needed
    twitterapi->resetConnections();

    emit resetUi();
    requestCount = 0;

    get();
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
  statusModel->setTheme( theme );
}

void Core::setModelData( TwitterAPI::SocialNetwork network, const QString &login )
{
  //TODO: debug, warning, etc.
  if ( login.isNull() )
    statusModel->clear();
  else
    statusModel->setStatusList( statusLists[ *accountsModel->account( network, login ) ] );
}

void Core::forceGet()
{
  timer->start();
  get();
}

void Core::get( TwitterAPI::SocialNetwork network, const QString &login, const QString &password )
{
//  requestCount = 0;
  twitterapi->friendsTimeline( network, login, password, settings.value("Appearance/tweet count", 20).toInt() );
  emit newRequest();
  if ( accountsModel->account( network, login )->directMessages ) {
    twitterapi->directMessages( network, login, password, settings.value("Appearance/tweet count", 20).toInt() );
    emit newRequest();
  }
}

void Core::get()
{
  bool started = false;
  foreach ( Account account, accountsModel->getAccounts() ) {
    if ( account.isEnabled ) {
      started = true;
      twitterapi->friendsTimeline( account.network, account.login, account.password, settings.value("Appearance/tweet count", 20).toInt());
      emit newRequest();
      if ( account.directMessages ) {
        twitterapi->directMessages( account.network, account.login, account.password, settings.value("Appearance/tweet count", 20).toInt());
        emit newRequest();
      }
    }
  }

  if ( started )
    emit requestStarted();
}

void Core::post( TwitterAPI::SocialNetwork network, const QString &login, const QString &status, quint64 inReplyTo )
{
  twitterapi->postUpdate( network, login, accountsModel->account( network, login )->password, status, inReplyTo );
  emit newRequest();
  emit requestStarted();
  checkForNew = false;
}

void Core::destroy( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, Entry::Type type )
{
  if ( settings.value( "General/confirmTweetDeletion", true ).toBool() ) {
    QMessageBox *confirm = new QMessageBox( QMessageBox::Warning,
                                            //: Are you sure to delete your message
                                            tr( "Are you sure?" ),
                                            tr( "Are you sure to delete this status?" ),
                                            QMessageBox::Yes | QMessageBox::Cancel,
                                            parentMainWindow );
    int result = confirm->exec();
    delete confirm;
    if ( result == QMessageBox::Cancel )
      return;
  }
  destroyDontAsk( network, login, id, type );
}

void Core::destroyDontAsk( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, Entry::Type type )
{
  checkForNew = false;
  if ( type == Entry::Status ) {
    twitterapi->deleteUpdate( network, login, accountsModel->account( network, login )->password, id );
  } else if ( type == Entry::DirectMessage ) {
    twitterapi->deleteDM( network, login, accountsModel->account( network, login )->password, id );
  }
  emit newRequest();
  emit requestStarted();
}

void Core::favoriteRequest( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited )
{
  qDebug() << "Core::favoriteRequest()";
  if ( favorited )
    twitterapi->createFavorite( network, login, accountsModel->account( network, login )->password, id );
  else
    twitterapi->destroyFavorite( network, login, accountsModel->account( network, login )->password, id );

  checkForNew = false;
  emit newRequest();
  emit requestStarted();
}

void Core::postDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &screenName, const QString &text )
{
  qDebug() << "Core::sendDM()";
  twitterapi->postDM( network, login, accountsModel->account( network, login )->password, screenName, text );
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

void Core::postDMDialog( TwitterAPI::SocialNetwork network, const QString &login, const QString &screenName )
{
  DMDialog *dlg = new DMDialog( network, login, screenName, parentMainWindow );
  connect( dlg, SIGNAL(dmRequest(TwitterAPI::SocialNetwork,QString,QString,QString)), this, SLOT(postDM(TwitterAPI::SocialNetwork,QString,QString,QString)) );
  connect( this, SIGNAL(confirmDMSent(TwitterAPI::SocialNetwork,QString,TwitterAPI::ErrorCode)), dlg, SLOT(showResult(TwitterAPI::SocialNetwork,QString,TwitterAPI::ErrorCode)) );

  dlg->exec();
  dlg->deleteLater();
}

Core::AuthDialogState Core::authDataDialog( Account *account )
{
  if ( authDialogOpen )
    return Core::STATE_DIALOG_OPEN;
  emit pauseIcon();
  QDialog *dlg = new QDialog( parentMainWindow );
  Ui::AuthDialog ui;
  ui.setupUi( dlg );
  //: This is for newly created account - when the login isn't given yet
  ui.loginEdit->setText( ( account->login == tr( "<empty>" ) ) ? QString() : account->login );
  ui.loginEdit->selectAll();
  ui.passwordEdit->setText( account->password );
  dlg->adjustSize();
  authDialogOpen = true;
  int row = accountsModel->indexOf( *account );
  if ( dlg->exec() == QDialog::Accepted ) {
    if ( ui.disableBox->isChecked() ) {
      authDialogOpen = false;
      account->isEnabled = false;
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
    if ( account->login != ui.loginEdit->text() ) {
      Account newAccount = *account;
      newAccount.login = ui.loginEdit->text();
      statusLists[ newAccount ] = statusLists[ *account ];
      statusLists[ newAccount ]->setLogin( newAccount.login );
      statusLists.remove( *account );
      account->login = ui.loginEdit->text();
      settings.setValue( QString("Accounts/%1/login").arg( accountsModel->indexOf( *account ) ), account->login );
      emit accountsUpdated( accountsModel->getAccounts() );
    }
    account->password = ui.passwordEdit->text();

    if ( settings.value( "General/savePasswords", Qt::Unchecked ).toInt() == Qt::Checked )
      settings.setValue( QString("Accounts/%1/password").arg( accountsModel->indexOf( *account ) ), ConfigFile::pwHash( account->password ) );

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
    statusModel->retranslateUi();
    if ( accounts )
      accounts->retranslateUi();
}

void Core::addEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry )
{
  if ( !statusLists.contains( *accountsModel->account( network, login ) ) )
    return;
  statusLists[ *accountsModel->account( network, login ) ]->addStatus( entry );

  if ( entry.type == Entry::Status ) {
    if ( imageDownload->contains( entry.userInfo.imageUrl ) ) {
      if ( !imageDownload->imageFromUrl( entry.userInfo.imageUrl )->isNull() )
        emit setImageForUrl( entry.userInfo.imageUrl, imageDownload->imageFromUrl( entry.userInfo.imageUrl ) );
    } else {
      imageDownload->imageGet( entry.userInfo.imageUrl );
    }
  }
}

void Core::deleteEntry( TwitterAPI::SocialNetwork network, const QString &login, quint64 id )
{
  Account *account = accountsModel->account( network, login );
  if ( statusLists.contains( *account ) ) {
    statusLists[ *account ]->deleteStatus( id );
    get( network, login, account->password );
  }
}

void Core::setFavorited( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited )
{
  Account *account = accountsModel->account( network, login );
  if ( statusLists.contains( *account ) ) {
    statusLists[ *account ]->setFavorited( id, favorited );
  }
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

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  requestCount--;
  if ( account->directMessages )
    requestCount--;
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_FRIENDS_TIMELINE ) )
    return;
  get( network, account->login, account->password );
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &status, quint64 inReplyToId )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  requestCount--;
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_POST_UPDATE ) )
    return;
  post( network, account->login, status, inReplyToId );
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &screenName, const QString &text )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  requestCount--;
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_POST_DM ) )
    return;
  postDM( network, account->login, screenName, text );
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 destroyId, Entry::Type type )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  requestCount--;
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_DELETE_UPDATE ) )
    return;
  destroyDontAsk( network, account->login, destroyId, type );
}

void Core::setupStatusLists()
{
  accountsModel->cleanUp();
  QList<Account> newAccounts = accountsModel->getAccounts();
  QMap<Account,QString> passwordMap;
  QString password;
  foreach ( Account account, newAccounts ) {
    password = account.password;
    account.password = QString();
    passwordMap.insert( account, password );
  }

  QList<Account> statusListsKeys = statusLists.keys();
  for( int i = 0; i < statusListsKeys.size(); ++i ) {
    Account &account = statusListsKeys[i];
    Account noPasswdAccount = account;
    noPasswdAccount.password = QString();
    if ( !passwordMap.keys().contains( noPasswdAccount ) ) {
      statusLists[ account ]->deleteLater();
      qDebug() << "Deleting statusList for account:" << account.login << account.network;
      statusLists.remove( account );
    } else {
      QList<Status> statuses = statusLists.value( account )->getData();
      int active = statusLists.value( account )->active();
      statusLists[ account ]->deleteLater();
      statusLists.remove( account );

      account.password = passwordMap.value( account );

      StatusList *statusList = new StatusList( account.login, account.network, this );
      statusList->setStatuses( statuses );
      statusList->setActive( active );
      statusLists.insert( account, statusList );
    }
  }

  foreach ( Account account, newAccounts ) {
    if ( account.isEnabled && !statusLists.contains( account ) ) {
      StatusList *statusList = new StatusList( account.login, account.network, this );
      statusLists.insert( account, statusList );
    }
    if ( !account.isEnabled && statusLists.contains( account ) ) {
      statusLists[ account ]->deleteLater();
      statusLists.remove( account );
    }
  }

  foreach ( Account account, newAccounts ) {
    if ( statusLists.contains( account ) )
      statusLists[ account ]->slotDirectMessagesChanged( account.directMessages );
  }

  requestCount = 0;
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
      break;
    case TwitterAPI::ROLE_DIRECT_MESSAGES:
      break;
    case TwitterAPI::ROLE_PUBLIC_TIMELINE:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
      break;
    }
  case Core::STATE_DIALOG_OPEN:
  case Core::STATE_REMOVE_ACCOUNT:
  case Core::STATE_DISABLE_ACCOUNT:
  default:;
  }
  return false;
}

void Core::slotNewRequest()
{
  requestCount++;
  qDebug() << requestCount << "request(s) pending";
}

void Core::resetRequestsCount()
{
  if ( requestCount > 0 ) {
    requestCount = 0;
    QMessageBox::warning( parentMainWindow, tr( "Warning" ),
                          tr( "One or more requests didn't complete. "
                              "Check your connection and/or accounts settings." ),
                          QMessageBox::Ok );
    qDebug() << "warning: some requests may failed...";
  }
}

void Core::addAccount()
{
  accounts->addAccount();
}

void Core::slotRequestDone( TwitterAPI::SocialNetwork network, const QString &login, int role )
{
  StatusList *statusList = statusModel->getStatusList();
  if ( statusList->network() == network
       && statusList->login() == login ){
    statusModel->updateDisplay();
  }
  if ( role != TwitterAPI::ROLE_POST_DM ) {
    requestCount--;
  }
  qDebug() << requestCount;
  if ( requestCount == 0 ) {
    tempModelCount = statusLists.count();
    if ( checkForNew )
      checkUnreadStatuses();
    emit resetUi();
  }
  checkForNew = true;
}

void Core::checkUnreadStatuses()
{
  QStringList unread;
  QString message;
  foreach ( Account account, statusLists.keys() ) {
    if ( statusLists[ account ]->hasUnread() ) {
      unread.append( QString( "%1 @%2" ).arg( account.login, Account::networkToString( account.network ) ) );
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

void Core::shortenUrl( const QString &url )
{
  urlShortener->shorten(url, (UrlShortener::Shortener) settings.value( "General/url-shortener" ).toInt() );
}

/*! \class Core
    \brief A class responsible for managing connections to Twitter.

    This class includes a high-level interface for connecting with Twitter API
    and submitting changes to the user's updates list. When the update
    is requested, an TwitterAPI class instance is created to perform the
    action. Once the received XML document is parsed, the ImageDownload
    class instance is engaged if necessary, to download profile images for
    new statuses. All the new Entries are passed to a StatusModel for displaying.
*/

/*! \enum Core::AuthDialogState
    \brief The return state of the authentication dialog.
*/

/*! \var Core::AuthDialogState Core::STATE_ACCEPTED
    Dialog was accepted.
*/

/*! \var Core::AuthDialogState Core::STATE_REJECTED
    Dialog was rejected.
*/

/*! \var Core::AuthDialogState Core::STATE_SWITCH_TO_PUBLIC
    User switched to public timeline syncing
*/

/*! \fn Core::Core( MainWindow *parent = 0 )
    Creates a Core class instance with a given \a parent.
*/

/*! \fn virtual Core::~Core()
    Virtual destructor.
*/

/*! \fn void Core::applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages )
    Sets the configuration given in Settings dialog and requests timeline update if necessary.
    \param msecs Timeline update interval
    \param user Authenticating user login
    \param password Authenticating user password
    \param publicTimeline Indicating whether to sync with public timeline or not
    \param directMessages Indicating whether to include direct messages when syncing
                          with friends timeline
*/

/*! \fn bool Core::setTimerInterval( int msecs )
    Sets timer interval to \a msecs miliseconds.
*/

/*! \fn void Core::setBrowserPath( const QString& path )
    Sets a path for the browser to be used to handle URL links opening.
    \param path Browser path.
*/

/*! \fn void Core::forceGet()
    Resets timer and enforces immediate timeline sync. This is to handle asynchronous sync requests, such as
    update button press, or changed settings. Normally the get method is used to update timeline always
    when timer emits timeout signal.
    \sa get()
*/

/*! \fn void Core::get()
    Issues a timeline sync request, either public or friends one (with or without direct messages), according to
    values returned by isPublicTimelineSync and isDirectMessagesSync. If necessary (when user's login and
    password are required and not provided, or when authorization fails) pops up an authentication dialog to get
    user authentication data.
    \sa post(), destroyStatus(), authDataDialog()
*/

/*! \fn void Core::post( const QByteArray &status, quint64 inReplyTo = -1 )
    Sends a new Status with a content given by \a status. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param status New Status's text.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
    \sa get(), destroyStatus(), authDataDialog()
*/

/*! \fn void Core::uploadPhoto( QString photoPath, QString status )
    Uploads a photo to TwitPic.com and, if \a status is not empty, posts a status update (this is done internally
    by TwitPic API).
    \param photoPath A path to photo to be uploaded.
    \param status New Status's text.
    \sa twitPicResponse(), get(), post()
*/

/*! \fn void Core::abortUploadPhoto()
    Interrupts uploading a photo to TwitPic.com.
    \sa uploadPhoto(), twitPicResponse()
*/

/*! \fn void Core::twitPicResponse( bool responseStatus, QString message, bool newStatus )
    Reads a response from TwitPic API.
    \param responseStatus true if photo was successfully uploaded, false otherwise.
    \param message Error message or URL to the uploaded photo, depending on a \a responseStatus.
    \param newStatus true if a new status was posted, false otherwise.
    \sa uploadPhoto()
*/

/*! \fn void Core::destroyStatus( int id )
    Sends a request to delete Status of id given by \a id. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param id Id of the Status to be deleted.
    \sa get(), post(), authDataDialog(), deleteEntry()
*/

/*! \fn void Core::downloadImage( Entry *entry )
    Downloads a profile image for the given \a entry. Creates an ImageDownload class instance
    and requests image from URL specfied inside \a entry.
    \param entry Entry containing a URL to requested image.
    \sa setImageForUrl()
*/

/*! \fn void Core::openBrowser( QUrl address )
    Opens a web browser with a given \a address. The browser opened is a system default browser
    on Mac and Windows. On Unix it's defined in Settings.
*/

/*! \fn Core::AuthDialogState Core::authDataDialog( const QString &user = QString(), const QString &password = QString() )
    Opens a dialog asking user for login and password to Twitter. Prevents opening a dialog when
    another instance is currently shown. Updates download-related flags and user's authentication
    data according to user's input.
    \param user User's login to show in dialog upon creation (default: empty string).
    \param password User's password to show in dialog upon creation (default: empty string).
    \returns Dialog's state.
    \sa AuthDialogState, getAuthData(), setAuthData()
*/

/*! \fn void Core::errorMessage( const QString &message )
    Sends a \a message to MainWindow class instance, to notify user about encountered
    problems. Works also as a proxy for internal ImageDownload and TwitterAPI classes instances.
    \param message Error message.
*/

/*! \fn void Core::addEntry( Entry* entry )
    Emitted when a single Status \a entry is parsed and ready to be inserted into model.
    \param entry Entry to insert into a model.
    \sa newEntry()
*/

/*! \fn void Core::deleteEntry( int id )
    Emitted when a positive response from Twitter API concerning destroying a Status is recieved
    and Status can be deleted form model.
    \param id Id of the Status.
    \sa destroyStatus()
*/

/*! \fn void Core::twitPicResponseReceived()
    Emitted when a response from TwitPic is received.
    \sa uploadPhoto(), abortUploadPhoto()
*/

/*! \fn void Core::twitPicDataSendProgress(int,int)
    Emitted when a response from TwitPic is received.
    \sa uploadPhoto(), abortUploadPhoto()
*/

/*! \fn void Core::setImageForUrl( const QString& url, QPixmap image )
    Emitted when an \a image is downloaded and is ready to be shown in model.
    \param url A URL pointing to \a image.
    \param image An image to show for Statuses with the given \a url
*/

/*! \fn void Core::requestStarted()
    Emitted when any of the post/get requests starts. Used to make MainWindow instance
    display the progress icon.
*/

/*! \fn void Core::resetUi()
    Emitted when TwitterAPI requests are finished, to notify MainWindow instance to
    reset StatusEdit field.
*/

/*! \fn void Core::timelineUpdated()
    Emitted to notify model that TwitterAPI requests are finished and notification popup
    can be displayed.
*/

/*! \fn void Core::publicTimelineSyncChanged( bool isEnabled )
    Emitted when public timeline syncronization is requested.
    \param isEnabled Indicates if public timeline was requested. If false, friends timeline will be downloaded.
    \sa setPublicTimelineSync()
*/
