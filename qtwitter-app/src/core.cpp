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
#include <twitterapi/twitterapi.h>
#include "core.h"
#include "settings.h"
#include "twitpicengine.h"
#include "tweetmodel.h"
#include "tweet.h"
#include "twitteraccountsmodel.h"
#include "urlshortener.h"
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"

extern ConfigFile settings;

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    authDialogOpen( false ),
    requestCount( 0 ),
    tempModelCount( 0 ),
    twitpicUpload( 0 ),
    timer( 0 ),
    parentMainWindow( parent )
{
  imageDownload = new ImageDownload( this );
  connect( imageDownload, SIGNAL(imageReadyForUrl(QString,QPixmap*)), this, SIGNAL(setImageForUrl(QString,QPixmap*)) );

  twitterapi = new TwitterAPI( this );
  connect( twitterapi, SIGNAL(newEntry(QString,Entry)), this, SLOT(addEntry(QString,Entry)) );
  connect( twitterapi, SIGNAL(deleteEntry(QString,int)), this, SLOT(deleteEntry(QString,int)) );
  connect( twitterapi, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( twitterapi, SIGNAL(unauthorized(QString,QString)), this, SLOT(slotUnauthorized(QString,QString)) );
  connect( twitterapi, SIGNAL(unauthorized(QString,QString,QString,int)), this, SLOT(slotUnauthorized(QString,QString,QString,int)) );
  connect( twitterapi, SIGNAL(unauthorized(QString,QString,int)), this, SLOT(slotUnauthorized(QString,QString,int)) );

  connect( this, SIGNAL(newRequest()), SLOT(slotNewRequest()) );
  connect( twitterapi, SIGNAL(requestDone(QString,int)), this, SLOT(slotRequestDone(QString,int)) );

  listViewForModels = parent->getListView();
  margin = parent->getScrollBarWidth();

  accountsModel = new TwitterAccountsModel( this );

  urlShortener = new UrlShortener( this );
  connect( urlShortener, SIGNAL(shortened(QString)), this, SIGNAL(urlShortened(QString)));
  connect( urlShortener, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));
}

Core::~Core()
{
  QMap<QString,TweetModel*>::iterator i = tweetModels.begin();
  while ( i != tweetModels.end() ) {
    (*i)->deleteLater();
    i++;
  }
}

void Core::applySettings()
{
//  setUrlShortener();
  publicTimeline = settings.value(  "TwitterAccounts/publicTimeline", false ).toBool();
  setupTweetModels();
  twitterapi->resetConnections();

  emit resetUi();
  requestCount = 0;

  int mtc = settings.value( "Appearance/tweet count", 25 ).toInt();
  foreach ( TweetModel *model, tweetModels.values() )
    model->setMaxTweetCount( mtc );
  foreach ( TwitterAccount account, accountsModel->getAccounts() ) {
    if ( tweetModels.contains( account.login ) )
      tweetModels[ account.login ]->slotDirectMessagesChanged( account.directMessages );
  }

  setTimerInterval( settings.value( "General/refresh-value", 15 ).toInt() * 60000 );
  get();
}

//void Core::setUrlShortener()
//{
//  if( urlShortener )
//    delete urlShortener;
//
//  switch( settings.value( "General/url-shortener" ).toInt() ) {
//    case UrlShortener::SHORTENER_ISGD:
//      urlShortener = new IsgdShortener( this );
//      break;
//    case UrlShortener::SHORTENER_TRIM:
//      urlShortener = new TrimShortener( this );
//      break;
//    case UrlShortener::SHORTENER_METAMARK:
//      urlShortener = new MetamarkShortener( this );
//      break;
//    case UrlShortener::SHORTENER_TINYURL:
//      urlShortener = new TinyurlShortener( this );
//      break;
//     case UrlShortener::SHORTENER_TINYARROWS:
//      urlShortener = new TinyarrowsShortener( this );
//      break;
//     case UrlShortener::SHORTENER_UNU:
//      urlShortener = new UnuShortener( this );
//  }
//  connect( urlShortener, SIGNAL(shortened(QString)), this, SIGNAL(urlShortened(QString)));
//  connect( urlShortener, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)));
//}

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
  foreach ( TweetModel *model, tweetModels.values() )
    model->setTheme( theme );
}

QAbstractItemModel* Core::getTwitterAccountsModel()
{
  return accountsModel;
}

TweetModel* Core::getModel( const QString &login )
{
  return tweetModels.contains( login ) ? tweetModels.value( login ) : 0;
}

TweetModel* Core::getPublicTimelineModel()
{
  return tweetModels.contains( TwitterAPI::PUBLIC_TIMELINE ) ? tweetModels.value( TwitterAPI::PUBLIC_TIMELINE ) : 0;
}

void Core::forceGet()
{
  timer->start();
  get();
}

void Core::get( const QString &login, const QString &password )
{
  twitterapi->friendsTimeline( login, password, settings.value("Appearance/tweet count", 20).toInt() );
  emit newRequest();
  if ( accountsModel->account( login )->directMessages ) {
    twitterapi->directMessages( login, password, settings.value("Appearance/tweet count", 20).toInt() );
    emit newRequest();
  }
}

void Core::get()
{
  foreach ( TwitterAccount account, accountsModel->getAccounts() ) {
    if ( account.isEnabled ) {
      twitterapi->friendsTimeline( account.login, account.password, settings.value("Appearance/tweet count", 20).toInt());
      emit newRequest();
      if ( account.directMessages ) {
        twitterapi->directMessages( account.login, account.password, settings.value("Appearance/tweet count", 20).toInt());
        emit newRequest();
      }
    }
  }

  if ( publicTimeline ) {
    twitterapi->publicTimeline();
    emit newRequest();
  }
  emit requestStarted();
}

void Core::post( const QString &login, const QString &status, int inReplyTo )
{
  twitterapi->postUpdate( login, accountsModel->account( login )->password, status, inReplyTo );
  emit newRequest();
  emit requestStarted();
}

void Core::destroyTweet( const QString &login, int id )
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
  twitterapi->deleteUpdate( login, accountsModel->account( login )->password, id );
  emit newRequest();
  emit requestStarted();
}

void Core::uploadPhoto( const QString &login, QString photoPath, QString status )
{
  twitpicUpload = new TwitPicEngine( this );
  qDebug() << "uploading photo";
  twitpicUpload->postContent( login, accountsModel->account( login )->password, photoPath, status );
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

Core::AuthDialogState Core::authDataDialog( TwitterAccount *account )
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
      emit twitterAccountsChanged( accountsModel->getAccounts(), publicTimeline );
      delete dlg;
      return Core::STATE_DISABLE_ACCOUNT;
    } else if ( ui.removeBox->isChecked() ) {
      authDialogOpen = false;
      accountsModel->removeRow( row );
      settings.deleteTwitterAccount( row, accountsModel->rowCount() );
      emit twitterAccountsChanged( accountsModel->getAccounts(), publicTimeline );
      delete dlg;
      return Core::STATE_REMOVE_ACCOUNT;
    }
    if ( account->login != ui.loginEdit->text() ) {
      tweetModels[ ui.loginEdit->text() ] = tweetModels[ account->login ];
      tweetModels[ ui.loginEdit->text() ]->setLogin( account->login );
      tweetModels.remove( account->login );
      account->login = ui.loginEdit->text();
      emit twitterAccountsChanged( accountsModel->getAccounts(), publicTimeline );
    }
    account->password = ui.passwordEdit->text();
    settings.setValue( QString("TwitterAccounts/%1/login").arg( accountsModel->indexOf( *account ) ), account->login );
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
  foreach ( TweetModel *model, tweetModels.values() ) {
    model->retranslateUi();
  }
}

void Core::addEntry( const QString &login, Entry entry )
{
  if ( !tweetModels.contains( login ) )
    return;

  tweetModels[ login ]->insertTweet( &entry );
  if ( entry.type == Entry::Status ) {
    if ( imageDownload->contains( entry.image ) ) {
      if ( imageDownload->imageFromUrl( entry.image )->isNull() )
        qDebug() << "image download in progress";
      else
        emit setImageForUrl( entry.image, imageDownload->imageFromUrl( entry.image ) );
    } else {
      imageDownload->imageGet( entry.image );
    }
  }
}

void Core::deleteEntry( const QString &login, int id )
{
  if ( tweetModels.contains( login ) )
    tweetModels[ login ]->deleteTweet( id );
}

void Core::slotUnauthorized( const QString &login, const QString &password )
{
  Q_UNUSED(password);
  TwitterAccount *account = accountsModel->account( login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_FRIENDS_TIMELINE ) )
    return;
  requestCount--;
  if ( account->directMessages )
    requestCount--;
  get( account->login, account->password );
}

void Core::slotUnauthorized( const QString &login, const QString &password, const QString &status, int inReplyToId )
{
  Q_UNUSED(password);
  TwitterAccount *account = accountsModel->account( login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_POST_UPDATE ) )
    return;
  requestCount--;
  post( account->login, status, inReplyToId );
}

void Core::slotUnauthorized( const QString &login, const QString &password, int destroyId )
{
  Q_UNUSED(password);
  TwitterAccount *account = accountsModel->account( login );
  if ( !retryAuthorizing( account, TwitterAPI::ROLE_DELETE_UPDATE ) )
    return;
  requestCount--;
  destroyTweet( account->login, destroyId );
}

void Core::setupTweetModels()
{
  foreach ( TwitterAccount account, accountsModel->getAccounts() ) {
    if ( account.isEnabled && !tweetModels.contains( account.login ) ) {
      TweetModel *model = new TweetModel( account.login, margin, listViewForModels, this );
      createConnectionsWithModel( model );
      tweetModels.insert( account.login, model );
    }
    if ( !account.isEnabled && tweetModels.contains( account.login ) ) {
      tweetModels[ account.login ]->deleteLater();
      tweetModels.remove( account.login );
    }
  }
  if ( publicTimeline && !tweetModels.contains( TwitterAPI::PUBLIC_TIMELINE ) ) {
    TweetModel *model = new TweetModel( TwitterAPI::PUBLIC_TIMELINE, margin, listViewForModels, this );
    createConnectionsWithModel( model );
    tweetModels.insert( TwitterAPI::PUBLIC_TIMELINE, model );
  }
  newTweets.clear();
  requestCount = 0;
}

void Core::createConnectionsWithModel( TweetModel *model )
{
  connect( model, SIGNAL(openBrowser(QUrl)), this, SLOT(openBrowser(QUrl)) );
  connect( model, SIGNAL(reply(QString,int)), this, SIGNAL(addReplyString(QString,int)) );
  connect( model, SIGNAL(about()), this, SIGNAL(about()) );
  connect( model, SIGNAL(destroy(QString,int)), this, SLOT(destroyTweet(QString,int)) );
  connect( model, SIGNAL(retweet(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( model, SIGNAL(newTweets(QString,bool)), this, SLOT(storeNewTweets(QString,bool)) );
  connect( this, SIGNAL(setImageForUrl(QString,QPixmap*)), model, SLOT(setImageForUrl(QString,QPixmap*)) );
  connect( this, SIGNAL(allRequestsFinished()), model, SLOT(checkForUnread()) );
  connect( this, SIGNAL(resizeData(int,int)), model, SLOT(resizeData(int,int)) );
}

bool Core::retryAuthorizing( TwitterAccount *account, int role )
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

void Core::slotRequestDone( const QString &login, int role )
{
  Q_UNUSED(login);
  Q_UNUSED(role);
  requestCount--;
  qDebug() << requestCount;
  if ( requestCount == 0 ) {
    tempModelCount = tweetModels.count();
    emit resetUi();
    emit allRequestsFinished();
  }
}

void Core::storeNewTweets( const QString &login, bool exists )
{
  if ( !tempModelCount )
    return;
  qDebug() << "Core::storeNewTweets( " + login + ", " + exists + " )";
  if ( exists )
    newTweets << login;
  if ( --tempModelCount == 0 && newTweets.size() > 0 )
    sendNewsInfo();
}

void Core::sendNewsInfo()
{
  QString message;
  if ( newTweets.count() == 1 ) {
    message.append( newTweets.at(0) );
    emit sendNewsReport( message );
    newTweets.clear();
    return;
  }
  message.append( newTweets.join( ", " ) );
  message.replace( message.lastIndexOf( ", " ), 2, QString( " %1 " ).arg( tr( "and" ) ) );
  emit sendNewsReport( tr( "For %1" ).arg(message) );
  newTweets.clear();
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
    new statuses. All the new Entries are passed to a TweetModel for displaying.
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
    \sa post(), destroyTweet(), authDataDialog()
*/

/*! \fn void Core::post( const QByteArray &status, int inReplyTo = -1 )
    Sends a new Tweet with a content given by \a status. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param status New Tweet's text.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
    \sa get(), destroyTweet(), authDataDialog()
*/

/*! \fn void Core::uploadPhoto( QString photoPath, QString status )
    Uploads a photo to TwitPic.com and, if \a status is not empty, posts a status update (this is done internally
    by TwitPic API).
    \param photoPath A path to photo to be uploaded.
    \param status New Tweet's text.
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

/*! \fn void Core::destroyTweet( int id )
    Sends a request to delete Tweet of id given by \a id. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param id Id of the Tweet to be deleted.
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
    Emitted when a single Tweet \a entry is parsed and ready to be inserted into model.
    \param entry Entry to insert into a model.
    \sa newEntry()
*/

/*! \fn void Core::deleteEntry( int id )
    Emitted when a positive response from Twitter API concerning destroying a Tweet is recieved
    and Tweet can be deleted form model.
    \param id Id of the Tweet.
    \sa destroyTweet()
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
    \param image An image to show for Tweets with the given \a url
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
