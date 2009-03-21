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


#ifndef CORE_H
#define CORE_H

#include <QStandardItemModel>
#include <QAuthenticator>
#include <QTimer>

#include "imagedownload.h"
#include "mainwindow.h"

class TwitPicEngine;
class TwitterAPI;

typedef QMap<QString, QImage> MapStringImage;

/*!
  \brief A class responsible for managing connections to Twitter.

  This class includes a high-level interface for connecting with Twitter API
  and submitting changes to the user's updates list. When the update
  is requested, an XmlDownload class instance is created to perform the
  action. Once the received XML document is parsed, the ImageDownload
  class instance is engaged if necessary, to download profile images for
  new statuses. All the new Entries are passed to a TweetModel for displaying.
*/
class Core : public QObject
{
  Q_OBJECT

public:

  /*!
    \brief The return state of the authentication dialog.
  */
  enum AuthDialogState {
    Accepted, /*!< Dialog was accepted */
    Rejected, /*!< Dialog was rejected */
    SwitchToPublic /*!< User switched to public timeline syncing */
  };

  /*!
    Creates a Core class instance with a given \a parent.
  */
  Core( MainWindow *parent = 0 );
  virtual ~Core(); /*!< Virtual destructor. */

  /*!
    Sets the configuration given in Settings dialog and requests timeline update if necessary.
    \param msecs Timeline update interval
    \param user Authenticating user login
    \param password Authenticating user password
    \param publicTimeline Indicating whether to sync with public timeline or not
    \param directMessages Indicating whether to include direct messages when syncing
                          with friends timeline
  */
  void applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages );

  bool setTimerInterval( int msecs ); /*!< Sets timer interval to \a msecs miliseconds. */

  /*!
    Sets whether the public timeline is requested.
    \param b If true, a sync with public timeline is requested. If false, a sync with friends timeline is performed.
    \sa isPublicTimelineSync()
  */
  bool setPublicTimelineSync( bool b );

  /*!
    Sets whether drect messages are requested, when syncing with friends timeline. This setting has no effect if isPublicTimelineSync returns true.
    \param b If true, direct messages are downloaded and added to friends timeline. If false, only friends' status updates are downloaded.
    \sa isDirectMessagesSync(), isPublicTimelineSync()
  */
  bool setDirectMessagesSync( bool b );


#ifdef Q_WS_X11
  /*!
    Sets a path for the browser to be used to handle URL links opening.
    \param path Browser path.
  */
  void setBrowserPath( const QString& path );
#endif

public slots:
  /*!
    Resets timer and enforces immediate timeline sync. This is to handle asynchronous sync requests, such as
    update button press, or changed settings. Normally the get method is used to update timeline always
    when timer emits timeout signal.
    \sa get()
  */
  void forceGet();

  /*!
    Issues a timeline sync request, either public or friends one (with or without direct messages), according to
    values returned by isPublicTimelineSync and isDirectMessagesSync. If necessary (when user's login and
    password are required and not provided, or when authorization fails) pops up an authentication dialog to get
    user authentication data.
    \sa post(), destroyTweet(), authDataDialog()
  */
  void get();

  /*!
    Sends a new Tweet with a content given by \a status. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param status New Tweet's text.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
    \sa get(), destroyTweet(), authDataDialog()
  */
  void post( const QByteArray &status, int inReplyTo = -1 );

  /*!
    Uploads a photo to TwitPic.com and, if \a status is not empty, posts a status update (this is done internally
    by TwitPic API).
    \param photoPath A path to photo to be uploaded.
    \param status New Tweet's text.
    \sa twitPicResponse(), get(), post()
  */
  void uploadPhoto( QString photoPath, QString status );

  /*!
    Interrupts uploading a photo to TwitPic.com.
    \sa uploadPhoto(), twitPicResponse()
  */
  void abortUploadPhoto();

  /*!
    Reads a response from TwitPic API.
    \param responseStatus true if photo was successfully uploaded, false otherwise.
    \param message Error message or URL to the uploaded photo, depending on a \a responseStatus.
    \param newStatus true if a new status was posted, false otherwise.
    \sa uploadPhoto()
  */
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );

  /*!
    Sends a request to delete Tweet of id given by \a id. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param id Id of the Tweet to be deleted.
    \sa get(), post(), authDataDialog(), deleteEntry()
  */
  void destroyTweet( int id );

  /*!
    Downloads a profile image for the given \a entry. Creates an ImageDownload class instance
    and requests image from URL specfied inside \a entry.
    \param entry Entry containing a URL to requested image.
    \sa setImageForUrl()
  */
  void downloadImage( Entry *entry );

  /*!
    Opens a web browser with a given \a address. The browser opened is a system default browser
    on Mac and Windows. On Unix it's defined in Settings.
  */
  void openBrowser( QUrl address );

  /*!
    Opens a dialog asking user for login and password to Twitter. Prevents opening a dialog when
    another instance is currently shown. Updates download-related flags and user's authentication
    data according to user's input.
    \param user User's login to show in dialog upon creation (default: empty string).
    \param password User's password to show in dialog upon creation (default: empty string).
    \returns Dialog's state.
    \sa AuthDialogState, getAuthData(), setAuthData()
  */
  AuthDialogState authDataDialog( const QString &user = QString(), const QString &password = QString() );

signals:
  /*!
    Sends a \a message to MainWindow class instance, to notify user about encountered
    problems. Works also as a proxy for internal ImageDownload and XmlDownload classes instances.
    \param message Error message.
  */
  void errorMessage( const QString &message );

  /*!
    Emitted when user authentication data changes.
    \param authenticator A QAuthenticator object containing new authentication data.
    \sa setAuthData(), authDataDialog()
  */
  void authDataSet( const QAuthenticator &authenticator );

  /*!
    Emitted when a single Tweet \a entry is parsed and ready to be inserted into model.
    \param entry Entry to insert into a model.
    \sa newEntry()
  */
  void addEntry( Entry* entry );

  /*!
    Emitted when a positive response from Twitter API concerning destroying a Tweet is recieved
    and Tweet can be deleted form model.
    \param id Id of the Tweet.
    \sa destroyTweet()
  */
  void deleteEntry( int id );

  /*!
    Emitted when a response from TwitPic is received.
    \sa uploadPhoto(), abortUploadPhoto()
  */
  void twitPicResponseReceived();

  /*!
    Emitted when a response from TwitPic is received.
    \sa uploadPhoto(), abortUploadPhoto()
  */
  void twitPicDataSendProgress(int,int);

  /*!
    Emitted when an \a image is downloaded and is ready to be shown in model.
    \param url A URL pointing to \a image.
    \param image An image to show for Tweets with the given \a url
  */
  void setImageForUrl( const QString& url, QImage image );

  /*!
    Emitted when user's request may possibly require deleting currently displayed list.
    \param isPublicTimeline Value returned by isPublicTimelineSync.
    \param isSwitchUser Indicates wether the user has changed since previous valid request.
  */
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);

  /*!
    Emitted when any of the post/get requests starts. Used to make MainWindow instance
    display the progress icon.
  */
  void requestStarted();

  /*!
    Emitted when XmlDownload requests are finished, to notify MainWindow instance to
    reset StatusEdit field.
  */
  void resetUi();

  /*!
    Emitted to notify model that XmlDownload requests are finished and notification popup
    can be displayed.
  */
  void timelineUpdated();

  /*!
    Emitted to notify model that direct messages have been disabled and are needed
    to be deleted from current view.
    \sa setDirectMessagesSync(), isDirectMessagesSync()
  */
  void noDirectMessages();
  void directMessagesSyncChanged( bool isEnabled );
  void publicTimelineSyncChanged( bool isEnabled );

private slots:
  void setImageInHash( const QString&, QImage );
  void slotUnauthorized();
  void slotUnauthorized( const QByteArray &status, int inReplyToId );
  void slotUnauthorized( int destroyId );

private:
  bool retryAuthorizing( int role );
  bool switchUser;
  bool authDialogOpen;
  TwitterAPI *twitterapi;
  TwitPicEngine *twitpicUpload;
  QMap<QString,ImageDownload*> imageDownloader;
  MapStringImage imageCache;
  QAuthenticator authData;
  QString currentUser;
  QTimer *timer;
#ifdef Q_WS_X11
  QString browserPath;
#endif
};


#endif //CORE_H
