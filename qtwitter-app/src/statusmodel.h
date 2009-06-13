/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#ifndef STATUSMODEL_H
#define STATUSMODEL_H

#include <QStandardItemModel>
#include <QUrl>
#include <twitterapi/twitterapi.h>

class QPixmap;
class StatusWidget;
class ThemeData;
class StatusModel;
class StatusListView;
class StatusList;
struct Status;

class StatusModel : public QStandardItemModel
{
  Q_OBJECT

public:

  enum StatusState {
    STATE_DISABLED,
    STATE_UNREAD,
    STATE_READ,
    STATE_ACTIVE
  };

  StatusModel( StatusListView *parentListView, QObject *parent = 0 );

  StatusWidget* currentStatus();
  void deselectCurrentIndex();
  void setTheme( const ThemeData &theme );
  void setStatusList( StatusList *statusList );
  StatusList * getStatusList() const;
  void setMaxStatusCount( int count );
  void populate();
  void clear();

public slots:
  void updateDisplay();
  void updateDisplay( int ind );
  void updateState( int ind );
  void updateImage( int ind );
  void removeStatus( int ind );
  void markAllAsRead();

  void sendDeleteRequest( quint64 id, Entry::Type type );
  void sendFavoriteRequest( quint64 id, bool favorited );
  void sendDMRequest( const QString &screenName );
  void selectStatus( const QModelIndex &index );
  void selectStatus( StatusWidget *status );
  void retranslateUi();
  void resizeData( int width, int oldWidth );
  void moveFocus( bool up );
  void setImageForUrl( const QString& url, QPixmap *image );

signals:
  void retweet( QString message );
  void destroy( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, Entry::Type type );
  void favorite( TwitterAPI::SocialNetwork network, const QString &login, quint64 id, bool favorited );
  void postDM( TwitterAPI::SocialNetwork network, const QString &login, const QString &screenName );
  void openBrowser( QUrl address );
  void reply( const QString &name, quint64 inReplyTo );
  void about();
  void markEverythingAsRead();

private slots:
  void emitOpenBrowser( QString address );

private:
  TwitterAPI::SocialNetwork network;
  QString login;
  StatusList *statusList;
  int maxStatusCount;
  QModelIndex currentIndex;
  StatusListView *view;
};

#endif // STATUSMODEL_H
