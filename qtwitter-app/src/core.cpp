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
#include <QDebug>
#include <urlshortener/urlshortener.h>
#include "core.h"
#include "mainwindow.h"
#include "imagedownload.h"
#include "settings.h"
#include "twitpicengine.h"
#include "statusmodel.h"
#include "statuswidget.h"
#include "accountsmodel.h"
#include "accountscontroller.h"
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"

extern ConfigFile settings;

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    authDialogOpen( false ),
    requestCount(0),
    tempModelCount(0),
    twitpicUpload(0),
    accounts(0),
    accountsModel(0),
    timer(0),
    parentMainWindow( parent )
{
  imageDownload = new ImageDownload( this );
  connect( imageDownload, SIGNAL(imageReadyForUrl(QString,QPixmap*)), this, SLOT(setImageForUrl(QString,QPixmap*)) );

  twitterapi = new TwitterAPIInterface( this );
  connect( twitterapi, SIGNAL(newEntry(TwitterAPI::SocialNetwork,QString,Entry)), this, SLOT(addEntry(TwitterAPI::SocialNetwork,QString,Entry)) );
  connect( twitterapi, SIGNAL(deleteEntry(TwitterAPI::SocialNetwork,QString,int)), this, SLOT(deleteEntry(TwitterAPI::SocialNetwork,QString,int)) );
  connect( twitterapi, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString,QString,int)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString,QString,int)) );
  connect( twitterapi, SIGNAL(unauthorized(TwitterAPI::SocialNetwork,QString,QString,int)), this, SLOT(slotUnauthorized(TwitterAPI::SocialNetwork,QString,QString,int)) );

  connect( this, SIGNAL(newRequest()), SLOT(slotNewRequest()) );
  connect( twitterapi, SIGNAL(requestDone(TwitterAPI::SocialNetwork,QString,int)), this, SLOT(slotRequestDone(TwitterAPI::SocialNetwork,QString,int)) );

  listViewForModels = parent->getListView();

  urlShortener = new UrlShortener( this );
  connect( urlShortener, SIGNAL(shortened(QString)), this, SIGNAL(urlShortened(QString)));
  connect( urlShortener, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));

  statusModel = new StatusModel( listViewForModels, this );
  parent->setListViewModel( statusModel );

  connect( statusModel, SIGNAL(openBrowser(QUrl)), this, SLOT(openBrowser(QUrl)) );
  connect( statusModel, SIGNAL(reply(QString,int)), this, SIGNAL(addReplyString(QString,int)) );
  connect( statusModel, SIGNAL(about()), this, SIGNAL(about()) );
  connect( statusModel, SIGNAL(destroy(TwitterAPI::SocialNetwork,QString,int)), this, SLOT(destroyStatus(TwitterAPI::SocialNetwork,QString,int)) );
  connect( statusModel, SIGNAL(retweet(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( statusModel, SIGNAL(newStatuses(QString,bool)), this, SLOT(storeNewStatuses(QString,bool)) );
  connect( this, SIGNAL(allRequestsFinished()), statusModel, SLOT(checkForUnread()) );
  connect( this, SIGNAL(resizeData(int,int)), statusModel, SLOT(resizeData(int,int)) );

}

Core::~Core()
{
}

void Core::createAccounts( QWidget *view )
{
  accounts = new AccountsController( view, this );
  if ( !accountsModel )
    accountsModel = accounts->getModel();
}

void Core::applySettings()
{
  static bool appStartup = true;
  publicTimeline = settings.value( "TwitterAccounts/publicTimeline", AccountsController::PT_NONE ).toInt();
  if ( appStartup )
    accounts->loadAccounts();
  appStartup = false;

  int mtc = settings.value( "Appearance/tweet count", 25 ).toInt();
  StatusList::setMaxCount( mtc );
  statusModel->setMaxStatusCount( mtc );

  setupStatusLists();
  emit accountsUpdated( accountsModel->getAccounts(), publicTimeline );

  // TODO: do this only when really needed
  twitterapi->resetConnections();

  emit resetUi();
  requestCount = 0;

  foreach ( Account account, accountsModel->getAccounts() ) {
    if ( statusLists.contains( account ) )
      statusLists[ account ]->slotDirectMessagesChanged( account.directMessages );
  }

  setTimerInterval( settings.value( "General/refresh-value", 15 ).toInt() * 60000 );
  get();
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
  else if ( login == TwitterAPI::PUBLIC_TIMELINE )
    statusModel->setStatusList( statusLists[ Account::publicTimeline( network ) ] );
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

  switch ( publicTimeline ) {
  case AccountsController::PT_BOTH:
  case AccountsController::PT_TWITTER:
    twitterapi->publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER );
    started = true;
    emit newRequest();
    if ( publicTimeline == AccountsController::PT_TWITTER )
      break;
  case AccountsController::PT_IDENTICA:
    twitterapi->publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA );
    started = true;
    emit newRequest();
  case AccountsController::PT_NONE:
  default:
    break;
  }

  if ( started )
    emit requestStarted();
}

void Core::post( TwitterAPI::SocialNetwork network, const QString &login, const QString &status, int inReplyTo )
{
  twitterapi->postUpdate( network, login, accountsModel->account( network, login )->password, status, inReplyTo );
  emit newRequest();
  emit requestStarted();
}

void Core::destroyStatus( TwitterAPI::SocialNetwork network, const QString &login, int id )
{
  if ( settings.value( "General/confirmTweetDeletion", true ).toBool() ) {
    QMessageBox *confirm = new QMessageBox( QMessageBox::Warning,
                                            //: Are you sure to delete your message
                                            tr( "Are you sure?" ),
                                            tr( "Are you sure to delete this tweet?" ),
                                            QMessageBox::Yes | QMessageBox::Cancel,
                                            parentMainWindow );
    int result = confirm->exec();
    delete confirm;
    if ( result == QMessageBox::Cancel )
      return;
  }
  twitterapi->deleteUpdate( network, login, accountsModel->account( network, login )->password, id );
  emit newRequest();
  emit requestStarted();
}

void Core::uploadPhoto( const QString &login, QString photoPath, QString status )
{
  twitpicUpload = new TwitPicEngine( this );
  qDebug() << "uploading photo";
  twitpicUpload->postContent( login, accountsModel->account( TwitterAPI::SOCIALNETWORK_TWITTER, login )->password, photoPath, status );
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
  emit resetUi();
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
      settings.setValue( QString("TwitterAccounts/%1/enabled").arg( row ), false );
      emit accountsUpdated( accountsModel->getAccounts(), publicTimeline );
      delete dlg;
      return Core::STATE_DISABLE_ACCOUNT;
    } else if ( ui.removeBox->isChecked() ) {
      authDialogOpen = false;
      accountsModel->removeRow( row );
      settings.deleteAccount( row, accountsModel->rowCount() );
      emit accountsUpdated( accountsModel->getAccounts(), publicTimeline );
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
      emit accountsUpdated( accountsModel->getAccounts(), publicTimeline );
    }
    account->password = ui.passwordEdit->text();

    settings.setValue( QString("TwitterAccounts/%1/login").arg( accountsModel->indexOf( *account ) ), account->login );
    if ( settings.value( "General/savePasswords", Qt::Unchecked ).toInt() == Qt::Checked )
      settings.setValue( QString("TwitterAccounts/%1/password").arg( accountsModel->indexOf( *account ) ), ConfigFile::pwHash( account->password ) );

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
}

void Core::addEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry )
{
  if ( login == TwitterAPI::PUBLIC_TIMELINE ) {
    if ( !statusLists.contains( Account::publicTimeline( network ) ) )
      return;
    statusLists[ Account::publicTimeline( network ) ]->addStatus( entry );
  } else {
    if ( !statusLists.contains( *accountsModel->account( network, login ) ) )
      return;
    statusLists[ *accountsModel->account( network, login ) ]->addStatus( entry );
  }

  if ( entry.type == Entry::Status ) {
    if ( imageDownload->contains( entry.userInfo.imageUrl ) ) {
      if ( !imageDownload->imageFromUrl( entry.userInfo.imageUrl )->isNull() )
        emit setImageForUrl( entry.userInfo.imageUrl, imageDownload->imageFromUrl( entry.userInfo.imageUrl ) );
    } else {
      imageDownload->imageGet( entry.userInfo.imageUrl );
    }
  }
}

void Core::deleteEntry( TwitterAPI::SocialNetwork network, const QString &login, int id )
{
  Account *account = accountsModel->account( network, login );
  if ( statusLists.contains( *account ) ) {
    statusLists[ *account ]->deleteStatus( id );
    get( network, login, account->password );
  }
}

void Core::setImageForUrl( const QString& url, QPixmap *image )
{
  Status status;
  foreach ( StatusList *statusList, statusLists )
  {
    for ( int i = 0; i < statusList->size(); i++ ) {
      status = statusList->data(i);
      if ( url == status.entry.userInfo.imageUrl ) {
//        status.image = *image;
        statusList->setImage( i, *image );
      }
    }
  }
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_FRIENDS_TIMELINE ) )
    return;
  requestCount--;
  if ( account->directMessages )
    requestCount--;
  get( network, account->login, account->password );
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &status, int inReplyToId )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_POST_UPDATE ) )
    return;
  requestCount--;
  post( network, account->login, status, inReplyToId );
}

void Core::slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int destroyId )
{
  Q_UNUSED(password);
  Account *account = accountsModel->account( network, login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_DELETE_UPDATE ) )
    return;
  requestCount--;
  destroyStatus( network, account->login, destroyId );
}

void Core::setupStatusLists()
{
  QList<Account> newAccounts = accountsModel->getAccounts();
  foreach ( Account account, statusLists.keys() ) {
    if ( account.login != TwitterAPI::PUBLIC_TIMELINE && !newAccounts.contains( account ) ) {
      statusLists[ account ]->deleteLater();
      statusLists.remove( account );
    }
  }

  foreach ( Account account, accountsModel->getAccounts() ) {
    if ( account.isEnabled && !statusLists.contains( account ) ) {
      StatusList *statusList = new StatusList( account.login, account.network, this );
      statusLists.insert( account, statusList );
//      model = new StatusModel( account.network, account.login, margin, listViewForModels, this );
//      createConnectionsWithModel( model );
//      statusModels.insert( account, model );
    }
    if ( !account.isEnabled && statusLists.contains( account ) ) {
      statusLists[ account ]->deleteLater();
      statusLists.remove( account );
    }
  }

  switch ( publicTimeline ) {
  case AccountsController::PT_NONE:
    if ( statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ) ) {
      statusLists[ Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ]->deleteLater();
      statusLists.remove( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) );
    }
    if ( statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) ) ) {
      statusLists[ Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ]->deleteLater();
      statusLists.remove( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) );
    }
    break;
  case AccountsController::PT_TWITTER:
    if ( !statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ) ) {
      StatusList *statusList = new StatusList( TwitterAPI::PUBLIC_TIMELINE, TwitterAPI::SOCIALNETWORK_TWITTER, this );
      statusLists.insert( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ), statusList );
    }
    if ( statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) ) ) {
      statusLists[ Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) ]->deleteLater();
      statusLists.remove( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) );
    }
    break;
  case AccountsController::PT_IDENTICA:
    if ( !statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) ) ) {
      StatusList *statusList = new StatusList( TwitterAPI::PUBLIC_TIMELINE, TwitterAPI::SOCIALNETWORK_IDENTICA, this );
      statusLists.insert( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ), statusList );
    }
    if ( statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ) ) {
      statusLists[ Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ]->deleteLater();
      statusLists.remove( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) );
    }
    break;
  case AccountsController::PT_BOTH:
    if ( !statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ) ) ) {
      StatusList *statusList = new StatusList( TwitterAPI::PUBLIC_TIMELINE, TwitterAPI::SOCIALNETWORK_TWITTER, this );
      statusLists.insert( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_TWITTER ), statusList );
    }
    if ( !statusLists.contains( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ) ) ) {
      StatusList *statusList = new StatusList( TwitterAPI::PUBLIC_TIMELINE, TwitterAPI::SOCIALNETWORK_IDENTICA, this );
      statusLists.insert( Account::publicTimeline( TwitterAPI::SOCIALNETWORK_IDENTICA ), statusList );
    }
  }
  newStatuses.clear();
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
    case TwitterAPI::ROLE_DELETE_UPDATE:
      emit errorMessage( tr( "Authentication is required to delete updates." ) );
    case TwitterAPI::ROLE_FRIENDS_TIMELINE:
    case TwitterAPI::ROLE_DIRECT_MESSAGES:
    case TwitterAPI::ROLE_PUBLIC_TIMELINE:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
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
  qDebug() << requestCount;
}

void Core::slotRequestDone( TwitterAPI::SocialNetwork network, const QString &login, int role )
{
  Q_UNUSED(role);
  StatusList *statusList = statusModel->getStatusList();
  if ( statusList->network() == network
       && statusList->login() == login ){
    statusModel->updateDisplay();
  }
  requestCount--;
  qDebug() << requestCount;
  if ( requestCount == 0 ) {
    tempModelCount = statusLists.count();
    emit resetUi();
    emit allRequestsFinished();
  }
}

void Core::storeNewStatuses( const QString &login, bool exists )
{
  if ( !tempModelCount )
    return;
  qDebug() << "Core::storeNewStatuses( " + login + ", " + exists + " )";
  if ( exists )
    newStatuses << login;
  if ( --tempModelCount == 0 && newStatuses.size() > 0 )
    sendNewsInfo();
}

void Core::sendNewsInfo()
{
  QString message;
  if ( newStatuses.count() == 1 ) {
    message.append( newStatuses.at(0) );
    emit sendNewsReport( message );
    newStatuses.clear();
    return;
  }
  message.append( newStatuses.join( ", " ) );
  message.replace( message.lastIndexOf( ", " ), 2, QString( " %1 " ).arg( tr( "and" ) ) );
  emit sendNewsReport( tr( "For %1" ).arg(message) );
  newStatuses.clear();
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

/*! \fn void Core::post( const QByteArray &status, int inReplyTo = -1 )
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

/*! \fn void Core::requestListRefresh( bool isPublicTimeline, bool isSwitchUser)
    Emitted when user's request may possibly require deleting currently displayed list.
    \param isPublicTimeline Value returned by isPublicTimelineSync.
    \param isSwitchUser Indicates wether the user has changed since previous valid request.
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

/*! \fn void Core::directMessagesSyncChanged( bool isEnabled )
    Emitted when direct messages downloading enabled state changes.
    \param isEnabled Indicates if direct messages were enabled or disabled.
    \sa setDirectMessagesSync(), isDirectMessagesSync()
*/

/*! \fn void Core::publicTimelineSyncChanged( bool isEnabled )
    Emitted when public timeline syncronization is requested.
    \param isEnabled Indicates if public timeline was requested. If false, friends timeline will be downloaded.
    \sa setPublicTimelineSync()
*/
