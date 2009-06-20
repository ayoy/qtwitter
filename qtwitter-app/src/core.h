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

  void setSettingsOpen( bool open );

public slots:
  void forceGet();
  void get();
  void get( TwitterAPI::SocialNetwork network, const QString &login, const QString &password );
  void post( TwitterAPI::SocialNetwork network, const QString &login, const QString &status, quint64 inReplyTo );
  void destroy( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, Entry::Type type );
  void favoriteRequest( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited );
  void postDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &screenName, const QString &text );

  void uploadPhoto( const QString &login, QString photoPath, QString status );
  void abortUploadPhoto();
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );

  void openBrowser( QUrl address );
  void shortenUrl( const QString &url );
  void resetRequestsCount();


  void retranslateUi();

signals:
  void accountsUpdated( const QList<Account> &accounts, int isPublicTimelineRequested );
  void errorMessage( const QString &message );
  void twitPicResponseReceived();
  void twitPicDataSendProgress(qint64,qint64);
  void requestListRefresh( bool isPublicTimeline, bool isSwitchUser);
  void requestStarted();
  void resetUi();
  void pauseIcon();
  void timelineUpdated();
  void directMessagesSyncChanged( bool b );
  void modelChanged( StatusModel *model );
  void addReplyString( const QString &user, quint64 id );
  void addRetweetString( QString message );
  void confirmDMSent( TwitterAPI::SocialNetwork network, const QString &login, TwitterAPI::ErrorCode error );
  void about();
  void sendNewsReport( QString message );
  void resizeData( int width, int oldWidth );
  void newRequest();
  void urlShortened( const QString &url);

private slots:
  void createAccounts( QWidget *view );
  void addEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry );
  void deleteEntry( TwitterAPI::SocialNetwork network, const QString &login, quint64 id );
  void setFavorited( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited = true );

  void postDMDialog( TwitterAPI::SocialNetwork network, const QString &login, const QString &screenName );
  AuthDialogState authDataDialog( Account *account );

  void setImageForUrl( const QString& url, QPixmap *image );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &status, quint64 inReplyToId );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, const QString &screenName, const QString &text );
  void slotUnauthorized( TwitterAPI::SocialNetwork network, const QString &login, const QString &password, quint64 destroyId, Entry::Type type );
  void slotNewRequest();
  void slotRequestDone( TwitterAPI::SocialNetwork network, const QString &login, int role );

  void setWaitForAccounts( bool wait );
  void markEverythingAsRead();

private:
  void destroyDontAsk( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, Entry::Type type );
  void setupStatusLists();
  void checkUnreadStatuses();
  bool retryAuthorizing( Account *account, int role );
  bool authDialogOpen;
  int publicTimeline;
  int requestCount;
  int tempModelCount;

  bool waitForAccounts;
  bool settingsOpen;
  bool checkForNew;

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
