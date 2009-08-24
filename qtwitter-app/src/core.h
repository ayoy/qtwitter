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

  enum CheckingForUnread {
    CheckForUnread,
    DontCheckForUnread
  };

  static Core* instance()
  {
    if ( !m_instance ) {
      qFatal( "Construct Core object first!" );
    }
    return m_instance;
  }


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
  inline void incrementRequestCount( CheckingForUnread check = CheckForUnread )
  {
    if ( m_requestCount == 0 || check == CheckForUnread ) {
      m_checkForUnread = check;
      emit requestStarted();
    }
    m_requestCount++;
  }
  inline void decrementRequestCount()
  {
    if ( m_requestCount > 0 ) {
      m_requestCount--;
    }
    if ( m_requestCount == 0 ) {
      if ( m_checkForUnread == CheckForUnread )
        checkUnreadStatuses();
      emit resetUi();
      m_checkForUnread = CheckForUnread;
    }
  }
  inline int requestCount() { return m_requestCount; }

  bool retryAuthorizing( Account *account, int role );

public slots:
  void forceGet();
  void get();
  void get( const QString &serviceUrl, const QString &login, const QString &password );
  void post( const QString &serviceUrl, const QString &login, const QString &status, quint64 inReplyTo );

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

  AuthDialogState authDataDialog( Account *account );

  void setImageForUrl( const QString& url, QPixmap *image );
  void slotNewRequest();
  void slotRequestDone( const QString &serviceUrl, const QString &login, int role );

  void setWaitForAccounts( bool wait );
  void markEverythingAsRead();

  void addAccount();

private:
  void storeSession();
  void restoreSession();
  void setupStatusLists();
  void checkUnreadStatuses();

  Account* findAccount( const Account &account );

  bool authDialogOpen;
  static int m_requestCount;
  static CheckingForUnread m_checkForUnread;

  bool waitForAccounts;
  bool settingsOpen;

  TwitPicEngine *twitpicUpload;
  UrlShortener *urlShortener;

  AccountsController *accounts;
  AccountsModel *accountsModel;

  QMap<Account*,StatusList*> statusLists;

  QTimer *timer;
  MainWindow *parentMainWindow;

  static Core *m_instance;

#ifdef Q_WS_X11
  QString browserPath;
#endif
};

#endif //CORE_H
