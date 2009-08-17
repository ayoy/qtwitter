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
class QStringList;
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

  bool setTimerInterval( int msecs );
#ifdef Q_WS_X11
  void setBrowserPath( const QString& path );
#endif

  void setModelTheme( const ThemeData &theme );
  void setModelData( const QString &serviceUrl, const QString &login );

  void setSettingsOpen( bool open );
  QStringList twitpicLogins() const;
  static inline void incrementRequestCount() { m_requestCount++; }
  static inline void decrementRequestCount() { if ( m_requestCount > 0 ) m_requestCount--; }
  static inline int requestCount() { return m_requestCount; }

  bool retryAuthorizing( Account *account, int role );

public slots:
  void forceGet();
  void get();
  void get( const QString &serviceUrl, const QString &login, const QString &password );
  void post( const QString &serviceUrl, const QString &login, const QString &status, quint64 inReplyTo );
  void destroy( const QString &serviceUrl, const QString &login, quint64 id, Entry::Type type );
  void favoriteRequest( const QString &serviceUrl, const QString &login, quint64 id, bool favorited );
  void postDM( const QString &serviceUrl, const QString &login, const QString &screenName, const QString &text );

  void uploadPhoto( const QString &login, QString photoPath, QString status );
  void abortUploadPhoto();
  void twitPicResponse( bool responseStatus, QString message, bool newStatus );

  void openBrowser( QUrl address );
  void shortenUrl( const QString &url );
  void resetRequestsCount();

  void applySettings();


  void retranslateUi();

signals:
  void accountsUpdated( const QList<Account> &accounts );
  void errorMessage( const QString &message );
  void twitPicResponseReceived();
  void twitPicDataSendProgress(qint64,qint64);
  void requestStarted();
  void resetUi();
  void pauseIcon();
  void timelineUpdated();
  void modelChanged( StatusModel *model );
  void confirmDMSent( TwitterAPI::SocialNetwork network, const QString &login, TwitterAPI::ErrorCode error );
  void sendNewsReport( QString message );
  void newRequest();
  void urlShortened( const QString &url);

  void accountDialogClosed( bool success );

private slots:
  void createAccounts( QWidget *view );

  void postDMDialog( const QString &serviceUrl, const QString &login, const QString &screenName );
  AuthDialogState authDataDialog( Account *account );

  void setImageForUrl( const QString& url, QPixmap *image );
  void slotNewRequest();
  void slotRequestDone( const QString &serviceUrl, const QString &login, int role );

  void setWaitForAccounts( bool wait );
  void markEverythingAsRead();

  void addAccount();

private:
//  void destroyDontAsk( const QString &serviceUrl, const QString &login, quint64 id, Entry::Type type );
  void setupStatusLists();
  void checkUnreadStatuses();
  bool authDialogOpen;
  static int m_requestCount;
  int tempModelCount;

  bool waitForAccounts;
  bool settingsOpen;
  bool checkForNew;

//  TwitterAPIInterface *twitterapi;

  TwitPicEngine *twitpicUpload;
  UrlShortener *urlShortener;

  AccountsController *accounts;
  AccountsModel *accountsModel;

  QMap<Account*,StatusList*> statusLists;

  QTimer *timer;
  MainWindow *parentMainWindow;

#ifdef Q_WS_X11
  QString browserPath;
#endif
};

#endif //CORE_H
