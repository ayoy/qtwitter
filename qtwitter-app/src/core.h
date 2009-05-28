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
#include <twitterapi/twitterapi.h>
#include "accountsmodel.h"
#include "statuslist.h"

class QAbstractItemModel;
class MainWindow;
class ImageDownload;
class TwitPicEngine;
class StatusModel;
class ThemeData;
class StatusListView;
class UrlShortener;
class AccountsController;

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
  void setModelData( TwitterAPI::SocialNetwork network, const QString &login );

public slots:
  void forceGet();
  void get();
  void get( TwitterAPI::SocialNetwork network, const QString &login, const QString &password );
  void post( TwitterAPI::SocialNetwork network, const QString &login, const QString &status, int inReplyTo );
  void destroyStatus( TwitterAPI::SocialNetwork network, const QString &login, int id );
  void favoriteRequest( TwitterAPI::SocialNetwork network, const QString &login, int id );

  void uploadPhoto( const QString &login, QString photoPath, QString status );
  void abortUploadPhoto();
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );

  void openBrowser( QUrl address );
  AuthDialogState authDataDialog( Account *account );
  void shortenUrl( const QString &url );

  void retranslateUi();

signals:
  void accountsUpdated( const QList<Account> &accounts, int isPublicTimelineRequested );
  void errorMessage( const QString &message );
  void twitPicResponseReceived();
  void twitPicDataSendProgress(int,int);
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);
  void requestStarted();
  void allRequestsFinished();
  void resetUi();
  void timelineUpdated();
  void directMessagesSyncChanged( bool b );
  void modelChanged( StatusModel *model );
  void addReplyString( const QString &user, int id );
  void addRetweetString( QString message );
  void about();
  void sendNewsReport( QString message );
  void resizeData( int width, int oldWidth );
  void newRequest();
  void urlShortened( const QString &url);

private slots:
  void createAccounts( QWidget *view );
  void addEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry );
  void deleteEntry( TwitterAPI::SocialNetwork network, const QString &login, int id );
  void setFavorited( TwitterAPI::SocialNetwork network, const QString &login, int id, bool favorite = true );

  void setImageForUrl( const QString& url, QPixmap *image );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &status, int inReplyToId );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, int destroyId );
  void slotNewRequest();
  void slotRequestDone( TwitterAPI::SocialNetwork network, const QString &login, int role );
  void storeNewStatuses( const QString &login, bool exists );

private:
  void sendNewsInfo();
  void setupStatusLists();
  bool retryAuthorizing( Account *account, int role );
  bool authDialogOpen;
  int publicTimeline;
  int requestCount;
  int tempModelCount;
  QStringList newStatuses;

  TwitterAPIInterface *twitterapi;

  TwitPicEngine *twitpicUpload;
  UrlShortener *urlShortener;

  ImageDownload* imageDownload;

  AccountsController *accounts;
  AccountsModel *accountsModel;

  StatusModel *statusModel;
  StatusListView *listViewForModels;

  QMap<Account,StatusList*> statusLists;

  QTimer *timer;
  MainWindow *parentMainWindow;

#ifdef Q_WS_X11
  QString browserPath;
#endif
};

#endif //CORE_H
