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


#ifndef TWITTERAPI_H
#define TWITTERAPI_H

#include <QAuthenticator>
#include <QObject>
#include "twitterapi_global.h"

class QTimer;

class XmlDownload;
class Entry;

class TWITTERAPISHARED_EXPORT TwitterAPI : public QObject
{
  Q_OBJECT

public:

  /*!
    Describes the function that the current connection has.
  */
  enum Role {
    Refresh, /*!< Statuses update is requested. */
    Submit, /*!< Posting a new status is requested. */
    Destroy /*!< Destroying a Tweet is requested. */
  };

  /*!
    Used to specify the content that is currently requested and has to be parsed.
  */
  enum ContentRequested {
    Statuses, /*!< Statuses are requested. */
    DirectMessages /*!< Direct messages are requested. */
  };

  TwitterAPI( QObject *parent = 0 );
  virtual ~TwitterAPI(); /*!< Virtual destructor. */

  bool isPublicTimelineSync(); /*!< Returns true if sync with public timeline is requested. \sa setPublicTimelineSync() */
  bool isDirectMessagesSync(); /*!< Returns true if direct messages downloading is requested. \sa setDirectMessagesSync() */

  /*!
    Sets user login and password for authentication at twitter.com.
    \param user User's login.
    \param password User's password.
  */
  bool setAuthData( const QString &user, const QString &password );

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

public slots:

  /*!
    Issues a timeline sync request, either public or friends one (with or without direct messages), according to
    values returned by isPublicTimelineSync and isDirectMessagesSync. If necessary (when user's login and
    password are required and not provided, or when authorization fails) pops up an authentication dialog to get
    user authentication data.
    \sa post(), destroyTweet(), authDataDialog()
  */
  bool get();

  /*!
    Sends a new Tweet with a content given by \a status. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param status New Tweet's text.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
    \sa get(), destroyTweet(), authDataDialog()
  */
  bool post( const QByteArray &status, int inReplyTo = -1 );

  /*!
    Sends a request to delete Tweet of id given by \a id. If user's authenticaton
    data is missing, pops up an authentication dialog.
    \param id Id of the Tweet to be deleted.
    \sa get(), post(), authDataDialog(), deleteEntry()
  */
  bool destroyTweet( int id );

  void abort();

  /*!
    Outputs user's login and password.
    \returns QAuthenticator object containing user's authentication data.
  */
  const QAuthenticator& getAuthData() const;

  /*!
    Used to figure out when XmlDownload instance finishes its job. XmlDownload class emits
    signals connected to this slot when it finishes its requests. When all the requests are
    finished (i.e. one request when public timeline is requested or direct messages downloading
    is disabled or two requests when friends timeline with direct messages is requested), this
    slot resets connections and notifies User of new Tweets.
    \sa timelineUpdated()
  */
  void setFlag( TwitterAPI::ContentRequested flag );

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

  void unauthorized();
  void unauthorized( const QByteArray &status, int inReplyToId );
  void unauthorized( int destroyId );

  void addEntry( Entry *entry );

  void deleteEntry( int id );

  /*!
    Emitted when user's request may possibly require deleting currently displayed list.
    \param isPublicTimeline Value returned by isPublicTimelineSync.
    \param isSwitchUser Indicates wether the user has changed since previous valid request.
  */
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);

  /*!
    Emitted when XmlDownload request is finished.
  */
  void done();

  /*!
    Emitted to notify model that XmlDownload requests are finished and notification popup
    can be displayed.
  */
  void timelineUpdated();

  /*!
    Emitted to notify model that direct messages have been disabled or enabled,
    according to \a isEnabled.
    \param isEnabled Indicates if direct messages were enabled or disabled.
    \sa setDirectMessagesSync(), isDirectMessagesSync()
  */
  void directMessagesSyncChanged( bool isEnabled );

  /*!
    Emitted when user switches to public timeline sync in authentication dialog.
    \sa isPublicTimelineSync(), setPublicTimelineSync()
  */
  void publicTimelineSyncChanged( bool isEnabled );

  void userChanged();

private slots:
  void newEntry( Entry* );

private:
  void createConnections( XmlDownload *xmlDownload );
  void destroyXmlConnection();
  bool publicTimelineSync;
  bool directMessagesSync;
  bool switchUser;
  bool authDialogOpen;
  XmlDownload *xmlGet;
  XmlDownload *xmlPost;
  QAuthenticator authData;
  QString currentUser;
  QTimer *timer;
  bool statusesDone;
  bool messagesDone;
};


#endif // TWITTERAPI_H
