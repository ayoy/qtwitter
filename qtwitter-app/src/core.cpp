/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <QSettings>
#include <QDesktopServices>
#include <QProcess>
#include "core.h"
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"
#include "twitterapi.h"
#include "twitpicengine.h"
#include "tweetmodel.h"
#include "twitteraccountsmodel.h"

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    authDialogOpen( false ),
    twitpicUpload( NULL ),
    timer( NULL )
{
  twitterapi = new TwitterAPI( this );
  connect( twitterapi, SIGNAL(addEntry(Entry*)), this, SIGNAL(addEntry(Entry*)) );
  connect( twitterapi, SIGNAL(addEntry(Entry*)), this, SLOT(downloadImage(Entry*)) );
  connect( twitterapi, SIGNAL(deleteEntry(int)), this, SIGNAL(deleteEntry(int)) );
  connect( twitterapi, SIGNAL(timelineUpdated()), this, SIGNAL(timelineUpdated()) );
  connect( twitterapi, SIGNAL(authDataSet(QAuthenticator)), this, SIGNAL(authDataSet(QAuthenticator)) );
  connect( twitterapi, SIGNAL(requestListRefresh(bool,bool)), this, SIGNAL(requestListRefresh(bool,bool)) );
  connect( twitterapi, SIGNAL(done()), this, SIGNAL(resetUi()) );
  connect( twitterapi, SIGNAL(unauthorized()), this, SLOT(slotUnauthorized()) );
  connect( twitterapi, SIGNAL(unauthorized(QByteArray,int)), this, SLOT(slotUnauthorized(QByteArray,int)) );
  connect( twitterapi, SIGNAL(unauthorized(int)), this, SLOT(slotUnauthorized(int)) );
  connect( twitterapi, SIGNAL(directMessagesSyncChanged(bool)), this, SIGNAL(directMessagesSyncChanged(bool)) );
  connect( twitterapi, SIGNAL(publicTimelineSyncChanged(bool)), this, SIGNAL(publicTimelineSyncChanged(bool)) );

  model = new TweetModel( parent->getScrollBarWidth(), parent->getListView(), this );
  connect( model, SIGNAL(openBrowser(QUrl)), this, SLOT(openBrowser(QUrl)) );
  connect( model, SIGNAL(reply(QString,int)), this, SIGNAL(addReplyString(QString,int)) );
  connect( model, SIGNAL(about()), this, SIGNAL(about()) );
  connect( model, SIGNAL(destroy(int)), this, SLOT(destroyTweet(int)) );
  connect( model, SIGNAL(retweet(QString)), this, SIGNAL(addRetweetString(QString)) );
  connect( model, SIGNAL(newTweets(int,QStringList,int,QStringList)), this, SIGNAL(newTweets(int,QStringList,int,QStringList)) );
  connect( this, SIGNAL(addEntry(Entry*)), model, SLOT(insertTweet(Entry*)) );
  connect( this, SIGNAL(deleteEntry(int)), model, SLOT(deleteTweet(int)) );
  connect( this, SIGNAL(setImageForUrl(QString,QImage)), model, SLOT(setImageForUrl(QString,QImage)) );
  connect( this, SIGNAL(requestListRefresh(bool,bool)), model, SLOT(setModelToBeCleared(bool,bool)) );
  connect( this, SIGNAL(timelineUpdated()), model, SLOT(sendNewsInfo()) );
  connect( this, SIGNAL(directMessagesSyncChanged(bool)), model, SLOT(slotDirectMessagesChanged(bool)) );
  connect( this, SIGNAL(resizeData(int,int)), model, SLOT(resizeData(int,int)) );

  accountsModel = new TwitterAccountsModel( this );
//  emit modelChanged( model );
}

Core::~Core() {}

void Core::applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages, int maxTweetCount )
{
  model->setMaxTweetCount( maxTweetCount );
  bool a = setTimerInterval( msecs );
  bool b = twitterapi->setAuthData( user, password );
  bool c = twitterapi->setPublicTimelineSync( publicTimeline );
  bool d = twitterapi->setDirectMessagesSync( directMessages );
  if ( a || b || c || (!c && d) )
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

bool Core::isPublicTimelineRequested() const
{
  return model->isPublicTimelineRequested();
}


void Core::setPublicTimelineRequested( bool b )
{
  model->setPublicTimelineRequested( b );
}

void Core::setModelTheme( const ThemeData &theme )
{
  model->setTheme( theme );
}

QAbstractItemModel* Core::getTwitterAccountsModel()
{
  return accountsModel;
}

void Core::forceGet()
{
  timer->start();
  get();
}

void Core::get()
{
  Core::AuthDialogState state;
  while ( !twitterapi->get() ) {
    state = authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() );
    switch ( state ) {
    case Core::Rejected:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
      return;
    case Core::SwitchToPublic:
      twitterapi->setPublicTimelineSync( true );
      emit requestListRefresh( twitterapi->isPublicTimelineSync(), false );
      break;
    default:;
    }
  }
  emit requestStarted();
}

void Core::post( const QByteArray &status, int inReplyTo )
{
  twitterapi->post( status, inReplyTo );
  emit requestStarted();
}

void Core::uploadPhoto( QString photoPath, QString status )
{
  if ( twitterapi->getAuthData().user().isEmpty() || twitterapi->getAuthData().password().isEmpty() ) {
    if ( authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() ) == Rejected ) {
      emit errorMessage( tr("Authentication is required to upload photos to TwitPic.") );
      return;
    }
  }
  twitpicUpload = new TwitPicEngine( this );
  qDebug() << "uploading photo";
  twitpicUpload->postContent( twitterapi->getAuthData(), photoPath, status );
}

void Core::abortUploadPhoto()
{
  if ( twitpicUpload ) {
    twitpicUpload->abort();
    twitpicUpload->deleteLater();
    twitpicUpload = NULL;
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
  twitpicUpload = NULL;
  QDialog dlg;
  Ui::TwitPicNewPhoto ui;
  ui.setupUi( &dlg );
  ui.textBrowser->setText( tr( "Photo available at:" ).append( " <a href=\"%1\">%1</a>" ).arg( message ) );
  dlg.exec();
}

void Core::destroyTweet( int id )
{
  twitterapi->destroyTweet( id );
  emit requestStarted();
}

void Core::downloadImage( Entry *entry )
{
  if ( entry->type == Entry::DirectMessage )
    return;

  if ( imageCache.contains( entry->image ) ) {
    if ( imageCache[ entry->image ].isNull() ) {
      qDebug() << "not downloading";
    } else {
      emit setImageForUrl( entry->image, imageCache[ entry->image ] );
    }
    return;
  }
  QString host = QUrl( entry->image ).host();
  if ( imageDownloader.contains( host ) ) {
    imageDownloader[host]->imageGet( entry );
    imageCache[ entry->image ] = QImage();
    qDebug() << "setting null image";
    return;
  }
  ImageDownload *getter = new ImageDownload();
  imageDownloader[host] = getter;
  connect( getter, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( getter, SIGNAL(imageReadyForUrl(QString,QImage)), this, SLOT(setImageInHash(QString,QImage)) );
  getter->imageGet( entry );
  imageCache[ entry->image ] = QImage();
  qDebug() << "setting null image" << imageCache[ entry->image ].isNull();
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

Core::AuthDialogState Core::authDataDialog( const QString &user, const QString &password )
{
  if ( authDialogOpen )
    return Accepted;
  emit resetUi();
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  ui.loginEdit->setText( ( user == QString() ) ? twitterapi->getAuthData().user() : user );
  ui.loginEdit->selectAll();
  ui.passwordEdit->setText( password );
  dlg.adjustSize();
  authDialogOpen = true;
  if (dlg.exec() == QDialog::Accepted) {
    if ( ui.publicBox->isChecked() ) {
      authDialogOpen = false;
      twitterapi->setPublicTimelineSync( true );
      emit requestListRefresh( twitterapi->isPublicTimelineSync(), false );
      emit requestStarted();
      return SwitchToPublic;
    }
    twitterapi->setAuthData( ui.loginEdit->text(), ui.passwordEdit->text() );
    emit authDataSet( twitterapi->getAuthData() );
    authDialogOpen = false;
    twitterapi->setPublicTimelineSync( false );
    emit requestListRefresh( twitterapi->isPublicTimelineSync(), true );
    emit requestStarted();
    return Accepted;
  }
  qDebug() << "returning false";
  authDialogOpen = false;
  return Rejected;
}

void Core::retranslateUi()
{
  model->retranslateUi();
}

void Core::setImageInHash( const QString &url, QImage image )
{
  imageCache[ url ] = image;
  emit setImageForUrl( url, image );
}

bool Core::retryAuthorizing( int role )
{
  Core::AuthDialogState state = authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() );
  switch ( state ) {
  case Core::Rejected:
    switch ( role ) {
    case TwitterAPI::Submit:
      emit errorMessage( tr( "Authentication is required to post updates." ) );
    case TwitterAPI::Destroy:
      emit errorMessage( tr( "Authentication is required to delete updates." ) );
    case TwitterAPI::Refresh:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
    }
    twitterapi->abort();
    return false;
  case Core::SwitchToPublic:
    twitterapi->abort();
    twitterapi->setPublicTimelineSync( true );
  default:;
  }
  return true;
}

void Core::slotUnauthorized()
{
  if ( !retryAuthorizing( TwitterAPI::Refresh ) )
    return;
  twitterapi->get();
}

void Core::slotUnauthorized( const QByteArray &status, int inReplyToId )
{
  if ( !retryAuthorizing( TwitterAPI::Submit ) )
    return;
  twitterapi->post( status, inReplyToId );
}

void Core::slotUnauthorized( int destroyId )
{
  if ( !retryAuthorizing( TwitterAPI::Destroy ) )
    return;
  twitterapi->destroyTweet( destroyId );
}

/*! \class Core
    \brief A class responsible for managing connections to Twitter.

    This class includes a high-level interface for connecting with Twitter API
    and submitting changes to the user's updates list. When the update
    is requested, an XmlDownload class instance is created to perform the
    action. Once the received XML document is parsed, the ImageDownload
    class instance is engaged if necessary, to download profile images for
    new statuses. All the new Entries are passed to a TweetModel for displaying.
*/

/*! \enum Core::AuthDialogState
    \brief The return state of the authentication dialog.
*/

/*! \var Core::AuthDialogState Core::Accepted
    Dialog was accepted.
*/

/*! \var Core::AuthDialogState Core::Rejected
    Dialog was rejected.
*/

/*! \var Core::AuthDialogState Core::SwitchToPublic
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
    problems. Works also as a proxy for internal ImageDownload and XmlDownload classes instances.
    \param message Error message.
*/

/*! \fn void Core::authDataSet( const QAuthenticator &authenticator )
    Emitted when user authentication data changes.
    \param authenticator A QAuthenticator object containing new authentication data.
    \sa setAuthData(), authDataDialog()
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

/*! \fn void Core::setImageForUrl( const QString& url, QImage image )
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
    Emitted when XmlDownload requests are finished, to notify MainWindow instance to
    reset StatusEdit field.
*/

/*! \fn void Core::timelineUpdated()
    Emitted to notify model that XmlDownload requests are finished and notification popup
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
