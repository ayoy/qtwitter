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

#include "entry.h"
#include "xmldownload.h"
#include "xmlparser.h"
#include "imagedownload.h"
#include "mainwindow.h"

typedef QMap<QString, QImage> MapStringImage;

class Core : public QObject {
  Q_OBJECT

public:

  enum AuthDialogState {
    Accepted,
    Rejected,
    SwitchToPublic
  };

  Core( MainWindow *parent = 0 );
  virtual ~Core();
  bool downloadsPublicTimeline();
  bool wantsDirectMessages();

// These 4 methods return a bool value indicating whether
// there is a need for updating Tweets, used later in applySettings()
  bool setTimerInterval( int msecs );
  bool setAuthData( const QString &user, const QString &password );
  bool setDownloadPublicTimeline( bool );
  bool setWantsDirectMessages( bool );

  void applySettings( int msecsTimeInterval, const QString &user, const QString &password, bool publicTimelineRequested, bool directMessagesRequested );
#ifdef Q_WS_X11
  void setBrowserPath( const QString& );
#endif

public slots:
  void get();
  void post( const QByteArray &status );

  AuthDialogState authDataDialog( const QString &user = QString(), const QString &password = QString() );
  const QAuthenticator& getAuthData() const;
  void storeCookie( const QStringList );
  void openBrowser( QString address = QString() );
  void downloadOneImage( Entry *entry );
  void destroyTweet( int id );
  void forceGet();
  void setFlag( XmlDownload::ContentRequested );

private slots:
  void setImageInHash( const QString&, QImage );
  void newEntry( Entry* );

signals:
  void errorMessage( const QString &message );
  void authDataSet( const QAuthenticator& );
  void switchToPublic();
  void xmlConnectionIdle();
  void addOneEntry( Entry* );
  void deleteEntry( int );
  void setImage( const Entry&, QImage );
  void setImageForUrl( const QString&, QImage );
  void requestListRefresh( bool, bool );
  void resetUi();
  void timelineUpdated();
  void noDirectMessages();

private:
  void destroyXmlConnection();
  bool downloadPublicTimeline;
  bool includeDirectMessages;
  bool userChanged;
  bool showingDialog;
  XmlDownload *xmlGet;
  XmlDownload *xmlPost;
  ImageDownload *imageDownload;
  QMap<QString,ImageDownload*> imagesGetter;
  MapStringImage imagesHash;
  QAuthenticator authData;
  QStringList cookie;
  QString currentUser;
  QTimer *timer;
  bool statusesFinished;
  bool messagesFinished;
#ifdef Q_WS_X11
  QString browserPath;
#endif
};


#endif //CORE_H
