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


#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include "statuslistview.h"
#include "statuslist.h"
#include "statuswidget.h"
#include "configfile.h"
#include "statusmodel.h"
#include "qtwitterapp.h"
#include "core.h"

extern ConfigFile settings;

StatusModel* StatusModel::m_instance = 0;

StatusModel* StatusModel::instance()
{
    if ( 0 == m_instance ) {
        m_instance = new StatusModel;
    }
    return m_instance;
}

StatusModel::StatusModel( QObject *parent ) :
        QStandardItemModel( 0, 0, parent ),
        statusList(0),
        maxStatusCount( 20 ),
        currentIndex( QModelIndex() ),
        m_displayMode( DisplayNames )
{
}

StatusModel::~StatusModel()
{
    m_instance = 0;
}

void StatusModel::connectView( StatusListView *listView )
{
    view = listView;
    view->setModel( this );
    connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectStatus(QModelIndex)) );
    connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
    connect( view, SIGNAL(moveFocusToUnread(bool)), this, SLOT(moveFocusToUnread(bool)) );
    connect( view, SIGNAL(deselectAll()), this, SLOT(deselectCurrentIndex()) );
}

void StatusModel::populate()
{
    if ( rowCount() < maxStatusCount ) {
        for ( int i = rowCount(); i < maxStatusCount; ++i ) {
            StatusWidget *widget = new StatusWidget;
            QStandardItem *newItem = new QStandardItem;
            newItem->setSizeHint( widget->size() );
            appendRow( newItem );
            view->setIndexWidget( newItem->index(), widget );
        }
    } else {
        removeRows( maxStatusCount, rowCount() - maxStatusCount );
    }
}

void StatusModel::updateDisplay()
{
    // statusList is meant to be the same size as model ( rowCount() ),
    // at the beginning we do have the statusList, but it's empty,
    // so rowCount() would fail
    if ( statusList ) {
        for ( int i = 0; i <  qMin(maxStatusCount, statusList->size() ); ++i ) {
            updateDisplay( i );
        }
    } else {
        for ( int i = 0; i < rowCount(); ++i ) {
            updateDisplay( i );
        }
    }
    // FIXME: Sorry, I know that it sucks... :|
    view->setUpdatesEnabled( false );
    view->resize( view->width(), view->height() - 1 );
    view->resize( view->width(), view->height() + 1 );
    view->setUpdatesEnabled( true );
}

void StatusModel::updateDisplay( int ind )
{
    StatusWidget *widget = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
    Q_ASSERT(widget);
    if ( statusList ) {
        widget->setStatusData( statusList->data( ind ) );
        if ( statusList->active() == ind ) {
            currentIndex = index( ind, 0 );
        }
    }
    item( ind )->setSizeHint( widget->size() );
}

void StatusModel::updateImage( int ind )
{
    StatusWidget *widget = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
    Q_ASSERT(widget);
    widget->setImage( statusList->data( ind ).image );
}

void StatusModel::updateState( int ind )
{
    StatusWidget *widget = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
    Q_ASSERT(widget);
    widget->setState( statusList->data( ind ).state );
}

void StatusModel::removeStatus( int ind )
{
    if ( ind == rowCount() - 1 )
        return;

    StatusWidget *widget;

    if ( currentIndex.isValid() ) {
        currentIndex = index( statusList->active(), 0 );
    }
    int i = ind;
    for ( ; i < statusList->size() - 1; ++i ) {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        widget->setStatusData( statusList->data(i + 1) );
    }
    for ( ; i < maxStatusCount; ++i ) {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        widget->initialize();
    }
}

StatusWidget* StatusModel::currentStatus()
{
    if ( !currentIndex.isValid() )
        return 0;

    return static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
}

void StatusModel::deselectCurrentIndex()
{
    if ( currentIndex.isValid() ) {
        StatusWidget *widget = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
        Q_ASSERT(widget);
        widget->setState( Status::Read );
        statusList->setState( currentIndex.row(), Status::Read );
        currentIndex = QModelIndex();
    }
}

void StatusModel::setTheme( const ThemeData &theme )
{
    StatusWidget::setTheme( theme );
    StatusWidget *widget;
    for ( int i = 0; i < rowCount(); i++ ) {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        Q_ASSERT(widget);
        widget->applyTheme();
    }
}

void StatusModel::setStatusList( StatusList *statusList )
{
    if ( this->statusList ) {
        this->statusList->setVisible( false );
        this->statusList->disconnect();
    }

    this->statusList = statusList;
    this->statusList->setVisible( true );
    connect( this->statusList, SIGNAL(statusAdded(int)), SLOT(updateDisplay(int)) );
    connect( this->statusList, SIGNAL(dataChanged(int)), SLOT(updateDisplay(int)) );
    connect( this->statusList, SIGNAL(statusDeleted(int)), SLOT(removeStatus(int)) );
    connect( this->statusList, SIGNAL(stateChanged(int)), SLOT(updateState(int)) );
    connect( this->statusList, SIGNAL(favoriteChanged(int)), SLOT(updateDisplay(int)) );
    connect( this->statusList, SIGNAL(imageChanged(int)), SLOT(updateImage(int)) );

    StatusWidget::setCurrentLogin( this->statusList->login() );
    StatusWidget::setCurrentServiceUrl( this->statusList->serviceUrl() );

    // for cleaning up the list when switching to public timeline that could have
    // less statuses than requested maximum
    if ( statusList->size() < maxStatusCount ) {
        StatusWidget *widget;
        for ( int i = statusList->size(); i < maxStatusCount; ++i ) {
            widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
            Q_ASSERT(widget);
            widget->initialize();
            item( i )->setSizeHint( widget->size() );
        }
    }

    int active = statusList->active();
    if ( active == -1 ) {
        currentIndex = QModelIndex();
        view->scrollTo( index( 0, 0 ) );
    } else {
        currentIndex = index( active, 0 );
        selectStatus( index( active, 0 ) );
        view->setCurrentIndex( currentIndex );
        view->scrollTo( currentIndex );
    }

    updateDisplay();
}

StatusList* StatusModel::getStatusList() const
{
    return statusList;
}

void StatusModel::setMaxStatusCount( int count )
{
    if ( statusList && count < maxStatusCount )
        statusList->remove( count, maxStatusCount - count );
    maxStatusCount = count;
    populate();
}

StatusModel::DisplayMode StatusModel::displayMode() const
{
    return m_displayMode;
}

void StatusModel::setDisplayMode( DisplayMode mode )
{
    if ( mode != m_displayMode ) {
        m_displayMode = mode;
        if ( statusList ) {
            StatusWidget *widget;
            for ( int i = 0; i < statusList->size() - 1; ++i ) {
                widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
                widget->setDisplayMode( m_displayMode );
            }
        }
    }
}

void StatusModel::clear()
{
    if ( !statusList )
        return;

    statusList = 0;

    StatusWidget *widget;
    for ( int i = 0; i < rowCount(); ++i )
    {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        Q_ASSERT(widget);
        widget->initialize();
    }
    updateDisplay();
}

void StatusModel::sendDeleteRequest( quint64 id, Entry::Type type )
{
    qDebug() << "StatusModel::sendDeleteRequest";
    if ( settings.value( "General/confirmTweetDeletion", true ).toBool() ) {
        QMessageBox *confirm = new QMessageBox( QMessageBox::Warning,
                                                //: Are you sure to delete your message
                                                tr( "Are you sure?" ),
                                                tr( "Are you sure to delete this status?" ),
                                                QMessageBox::Yes | QMessageBox::Cancel,
                                                QTwitterApp::instance()->activeWindow() );
        int result = confirm->exec();
        delete confirm;
        if ( result == QMessageBox::Cancel )
            return;
    }
    statusList->requestDestroy( id, type );
    //  emit requestStarted();
}

void StatusModel::sendFavoriteRequest( quint64 id, bool favorited )
{
    qDebug() << "StatusModel::sendFavoriteRequest";
    if ( favorited ) {
        statusList->requestCreateFavorite( id );
    } else {
        statusList->requestDestroyFavorite( id );
    }
    //  emit requestStarted();
}

void StatusModel::sendDMRequest( const QString &screenName )
{
    qDebug() << "StatusModel::sendDMRequest";
    statusList->postDMDialog( screenName );
}

void StatusModel::selectStatus( const QModelIndex &index )
{
    if ( !statusList || !index.isValid() )
        return;

    // workaround for scrolling public timeline (limited to 20 statuses per call )
    // with maximum status count set to >20
    if ( index.row() >= statusList->size() ) {
        view->setCurrentIndex( index );
        return;
    }

    Status::State state;
    StatusWidget *widget;

    if ( currentIndex.isValid() ) {
        state = statusList->state( currentIndex.row() );
        if ( state != Status::Unread ) {
            state = Status::Read;
            statusList->setState( currentIndex.row(), state );

            widget = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
            Q_ASSERT(widget);
            widget->setState( state );
        }
    }

    currentIndex = index;

    statusList->setState( index.row(), Status::Active );

    widget = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
    Q_ASSERT(widget);
    widget->setState( Status::Active );

    view->setCurrentIndex( currentIndex );
}

void StatusModel::selectStatus( StatusWidget *statusWidget )
{
    if ( !statusList || statusWidget->getState() == Status::Disabled )
        return;

    Status status;
    StatusWidget *widget;
    if ( currentIndex.isValid() && currentIndex.row() < statusList->size() ) {
        status = statusList->data( currentIndex.row() );
        if ( status.state != Status::Unread ) {
            statusList->setState( currentIndex.row(), Status::Read );
            widget = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
            Q_ASSERT(widget);
            widget->setState( Status::Read );
        }
    }
    for ( int i = 0; i < statusList->size(); i++ ) {
        status = statusList->data( i );
        if ( status.entry.id == statusWidget->getId() ) {
            currentIndex = item(i)->index();
            statusList->setState( currentIndex.row(), Status::Active );
            statusWidget->setState( Status::Active );
            view->setCurrentIndex( currentIndex );
        }
    }
}

void StatusModel::markAllAsRead()
{
    int count = qMin( statusList->size(), rowCount() );

    if ( count > 0 ) {
        Status status;
        for ( int i = 0; i < count; i++ ) {
            status = statusList->data(i);
            if ( i == currentIndex.row() )
                status.state = Status::Active;
            else
                status.state = Status::Read;
            statusList->setState(i, status.state );
        }
    }
}

void StatusModel::retranslateUi()
{
    StatusWidget *widget;
    for ( int i = 0; i < rowCount(); i++ ) {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        widget->retranslateUi();
    }
}

void StatusModel::resizeData( int width, int oldWidth )
{
    Q_UNUSED(oldWidth);

    StatusWidget *widget;
    for ( int i = 0; i < rowCount(); i++ ) {
        widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
        widget->resize( width, widget->size().height() );
        item(i)->setSizeHint( widget->size() );
    }
}

void StatusModel::moveFocus( bool up )
{
    if ( !rowCount() )
        return;
    if ( !currentIndex.isValid() ) {
        currentIndex = index( 0, 0 );
        selectStatus( currentIndex );
        return;
    }
    if ( up ) {
        if ( currentIndex.row() > 0 ) {
            selectStatus( currentIndex.sibling( currentIndex.row() - 1, 0 ) );
        }
    } else {
        if ( currentIndex.row() < rowCount() - 1 ) {
            selectStatus( currentIndex.sibling( currentIndex.row() + 1, 0 ) );
        }
    }
}

void StatusModel::moveFocusToUnread( bool up )
{
    int diff;
    int start = currentIndex.row();
    int row;
    int count = qMin( statusList->size(), rowCount() );

    if ( !rowCount() )
        return;
    if ( up ) {
        if ( !currentIndex.isValid() ) {
            start = count - 1;
        }
        diff = -1;
    } else {
        if ( !currentIndex.isValid() ) {
            start = 0;
        }
        diff = 1;
    }

    row = start;

    do {
        Status::State state;
        state = statusList->state( row );
        if ( state != Status::Unread ) {
            row += diff;
            if ( row < 0 ) {
                row = count - 1;
            }
            if ( row >= count ) {
                row = 0;
            }
        } else {
            break;
        }
    } while ( row != start );

    if (row != start) {
        selectStatus( currentIndex.sibling( row, 0 ) );
    }
}

void StatusModel::setImageForUrl( const QString& url, QPixmap *image )
{
    Status status;
    StatusWidget *widget;
    for ( int i = 0; i < statusList->size(); i++ ) {
        status = statusList->data(i);
        if ( url == status.entry.userInfo.imageUrl ) {
            status.image = *image;
            widget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
            widget->setImage( *image );
        }
    }
}

void StatusModel::emitOpenBrowser( QString address )
{
    emit openBrowser( QUrl( address ) );
}
