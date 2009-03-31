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
class TweetModel;

typedef QMap<QString, QImage> MapStringImage;

class Core : public QObject
{
  Q_OBJECT

public:
  enum AuthDialogState {
    Accepted,
    Rejected,
    SwitchToPublic
  };

  Core( MainWindow *parent = 0 );
  virtual ~Core();

  void applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages, int maxTweetCount );
  bool setTimerInterval( int msecs );
#ifdef Q_WS_X11
  void setBrowserPath( const QString& path );
#endif

  void setPublicTimelineRequested( bool b );
  void setModelTheme( const ThemeData &theme );

public slots:
  void forceGet();
  void get();
  void post( const QByteArray &status, int inReplyTo = -1 );

  void uploadPhoto( QString photoPath, QString status );
  void abortUploadPhoto();
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );
  void destroyTweet( int id );

  void downloadImage( Entry *entry );
  void openBrowser( QUrl address );
  AuthDialogState authDataDialog( const QString &user = QString(), const QString &password = QString() );

  void retranslateUi();
  inline TweetModel* getModel() { return model; }

signals:
  void errorMessage( const QString &message );
  void authDataSet( const QAuthenticator &authenticator );
  void addEntry( Entry* entry );
  void deleteEntry( int id );
  void twitPicResponseReceived();
  void twitPicDataSendProgress(int,int);
  void setImageForUrl( const QString& url, QImage image );
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);
  void requestStarted();
  void resetUi();
  void timelineUpdated();
  void directMessagesSyncChanged( bool isEnabled );
  void publicTimelineSyncChanged( bool isEnabled );
  void modelChanged( TweetModel *model );
  void addReplyString( const QString &user, int id );
  void addRetweetString( QString message );
  void about();
  void newTweets( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames );
  void resizeData( int width, int oldWidth );

private slots:
  void setImageInHash( const QString&, QImage );
  void slotUnauthorized();
  void slotUnauthorized( const QByteArray &status, int inReplyToId );
  void slotUnauthorized( int destroyId );

private:
  bool retryAuthorizing( int role );
  bool authDialogOpen;
  TwitterAPI *twitterapi;
  TwitPicEngine *twitpicUpload;
  QMap<QString,ImageDownload*> imageDownloader;
  TweetModel *model;
  QMap<QString,TweetModel> models;
  MapStringImage imageCache;
  QAuthenticator authData;
  QString currentUser;
  QTimer *timer;
#ifdef Q_WS_X11
  QString browserPath;
#endif
};

#endif //CORE_H
