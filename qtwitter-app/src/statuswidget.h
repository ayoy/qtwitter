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


#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QtGui/QWidget>
#include "statusmodel.h"

class QMenu;
class QSignalMapper;
class Entry;
class ThemeData;
class Status;

namespace Ui {
    class StatusWidget;
}

class StatusWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY( StatusWidget )

public:

    explicit StatusWidget( QWidget *parent = 0 );
    virtual ~StatusWidget();

    const Entry& data() const;
    void resize( const QSize& size );
    void resize( int w, int h );

    void initialize();
    void setStatusData( const Status &status );
    void setImage( const QPixmap &pixmap );
    void setState( StatusModel::StatusState state );
    void setDisplayMode( StatusModel::DisplayMode mode );
    StatusModel::StatusState getState() const;

    static ThemeData getTheme();
    static void setTheme( const ThemeData &theme );

    void applyTheme();
    void retranslateUi();

    quint64 getId() const;

    static void setScrollBarWidth( int width );
    static void setCurrentWidth( int width );
    static void setCurrentLogin( const QString &login );
    static void setCurrentServiceUrl( const QString &serviceUrl );

public slots:
    void slotReply();
    void slotRetweet();
    void slotDM();
    void slotCopyLink();
    void slotDelete();
    void slotFavorite();
    void adjustSize();

signals:
    void reply( const QString &name, quint64 inReplyTo );
    void retweet( QString message );
    void markAllAsRead();
    void selectMe( StatusWidget *status );
    void deleteStatus( quint64 id );
    void getUserInfo( int userId);

protected:
    void changeEvent( QEvent *e );
    void enterEvent( QEvent *e );
    void leaveEvent( QEvent *e );
    void mousePressEvent( QMouseEvent *e );

private slots:
    void focusRequest();
    void handleReplyDeleteButton();

private:
    void createMenu();
    void setupMenu();
    QMenu *menu;

    QAction *replyAction;
    QAction *retweetAction;
    QAction *dmAction;
    QAction *copylinkAction;
    QAction *markallasreadAction;
    QAction *markeverythingasreadAction ;
    QAction *gotohomepageAction;
    QAction *gototwitterpageAction;
    QAction *deleteAction;
    QAction *favoriteAction;

    StatusModel::StatusState statusState;
    const Entry *statusData;

    static int scrollBarWidth;
    static int currentWidth;
    static ThemeData currentTheme;
    static QString currentLogin;
    static QString currentServiceUrl;
    static StatusWidget *activeStatus;

    QSignalMapper *signalMapper;
    Ui::StatusWidget *m_ui;
};

#endif // STATUSWIDGET_H
