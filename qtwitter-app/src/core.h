/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#ifndef CORE_H
#define CORE_H

#include <QStandardItemModel>
#include <QAuthenticator>
#include <QTimer>
#include <QMap>
#include <QCache>
#include "imagedownload.h"
#include "mainwindow.h"

class TwitPicEngine;
class TweetModel;
class QAbstractItemModel;
class TwitterAccountsModel;
class StatusList;
class TwitterAPI;
class UrlShortener;

class Core : public QObject
{
  Q_OBJECT

public:
  enum AuthDialogState {
    STATE_ACCEPTED,
    STATE_REJECTED,
    STATE_DIALOG_OPEN,
    STATE_DISABLE_ACCOUNT,
    STATE_REMOVE_ACCOUNT
  };

  Core( MainWindow *parent = 0 );
  virtual ~Core();

  void applySettings();
  bool setTimerInterval( int msecs );
#ifdef Q_WS_X11
  void setBrowserPath( const QString& path );
#endif

  void setModelTheme( const ThemeData &theme );
  QAbstractItemModel* getTwitterAccountsModel();
  TweetModel* getModel( const QString &login );
  TweetModel* getPublicTimelineModel();

public slots:
  void forceGet();
  void get();
  void get( const QString &login, const QString &password );
  void post( const QString &login, const QString &status, int inReplyTo );
  void destroyTweet( const QString &login, int id );

  void uploadPhoto( const QString &login, QString photoPath, QString status );
  void abortUploadPhoto();
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );

  void openBrowser( QUrl address );
  AuthDialogState authDataDialog( TwitterAccount *account );
  void shortenUrl( const QString &url );

  void retranslateUi();

signals:
  void twitterAccountsChanged( const QList<TwitterAccount> &accounts, bool isPublicTimelineRequested );
  void errorMessage( const QString &message );
  void twitPicResponseReceived();
  void twitPicDataSendProgress(int,int);
  void setImageForUrl( const QString& url, QPixmap *image );
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);
  void requestStarted();
  void allRequestsFinished();
  void resetUi();
  void timelineUpdated();
  void directMessagesSyncChanged( bool b );
  void modelChanged( TweetModel *model );
  void addReplyString( const QString &user, int id );
  void addRetweetString( QString message );
  void about();
  void sendNewsReport( QString message );
  void resizeData( int width, int oldWidth );
  void newRequest();
  void urlShortened( const QString &url);

private slots:
  void addEntry( const QString &login, Entry entry );
  void deleteEntry( const QString &login, int id );
  void slotUnauthorized( const QString &login, const QString &password );
  void slotUnauthorized( const QString &login, const QString &password, const QString &status, int inReplyToId );
  void slotUnauthorized( const QString &login, const QString &password, int destroyId );
  void slotNewRequest();
  void slotRequestDone( const QString &login, int role );
  void storeNewTweets( const QString &login, bool exists );

private:
  void sendNewsInfo();
  void setupTweetModels();
  void createConnectionsWithModel( TweetModel *model );
//  void setUrlShortener();
  bool retryAuthorizing( TwitterAccount *account, int role );
  bool authDialogOpen;
  bool publicTimeline;
  int requestCount;
  int tempModelCount;
  QStringList newTweets;

  TwitPicEngine *twitpicUpload;
  UrlShortener *urlShortener;

  ImageDownload* imageDownload;

  TwitterAccountsModel *accountsModel;
  TwitterAPI *twitterapi;
  QMap<QString,TweetModel*> tweetModels;
  QTimer *timer;

  StatusList *listViewForModels;
  MainWindow *parentMainWindow;
  int margin;

#ifdef Q_WS_X11
  QString browserPath;
#endif
};

#endif //CORE_H
