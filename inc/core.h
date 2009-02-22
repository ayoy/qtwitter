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
  void setTimerInterval( int msecs );
#ifdef Q_WS_X11
  void setBrowserPath( const QString& );
#endif

public slots:
  void get();
  void post( const QByteArray &status );

  AuthDialogState authDataDialog( const QString &user = QString(), const QString &password = QString() );
  void setAuthData( const QString &user, const QString &password );
  const QAuthenticator& getAuthData() const;
  void storeCookie( const QStringList );
  void setDownloadPublicTimeline( bool );
  void openBrowser( QString address = QString() );
  void downloadOneImage( Entry *entry );
  void destroyTweet( int id );
  void forceGet();

private slots:
  void destroyXmlConnection();
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
  void requestListRefresh( bool downloadPublicTimeline );
  void resetUi();

private:
  bool downloadPublicTimeline;
  bool showingDialog;
  XmlDownload *xmlGet;
  XmlDownload *xmlPost;
  ImageDownload *imageDownload;
  QMap<QString,ImageDownload*> imagesGetter;
  MapStringImage imagesHash;
  QAuthenticator authData;
  QStringList cookie;
#ifdef Q_WS_X11
  QString browserPath;
#endif
  QTimer *timer;
};


#endif //CORE_H
