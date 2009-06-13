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
#include "statuslistview.h"
#include "statuslist.h"
#include "statuswidget.h"
#include "configfile.h"
#include "statusmodel.h"

StatusModel::StatusModel( StatusListView *parentListView, QObject *parent ) :
    QStandardItemModel( 0, 0, parent ),
    statusList(0),
    maxStatusCount( 20 ),
    currentIndex( QModelIndex() ),
    view( parentListView )
{
  connect( view, SIGNAL(clicked(QModelIndex)), this, SLOT(selectStatus(QModelIndex)) );
  connect( view, SIGNAL(moveFocus(bool)), this, SLOT(moveFocus(bool)) );
}

void StatusModel::populate()
{
  if ( rowCount() < maxStatusCount ) {
    for ( int i = rowCount(); i < maxStatusCount; ++i ) {
      StatusWidget *status = new StatusWidget( this );
      QStandardItem *newItem = new QStandardItem;
      newItem->setSizeHint( status->size() );
      appendRow( newItem );
      view->setIndexWidget( newItem->index(), status );
    }
  } else {
    removeRows( maxStatusCount, rowCount() - maxStatusCount );
  }
}

void StatusModel::updateDisplay()
{
  // statusList is meant to be the same size as model ( rowCount() ),
  // but at the beginning we do have the statusList, but it's empty,
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
  StatusWidget *status = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(status);
  if ( statusList )
    status->setStatusData( statusList->data( ind ) );
  item( ind )->setSizeHint( status->size() );
}

void StatusModel::updateImage( int ind )
{
  StatusWidget *status = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(status);
  status->setImage( statusList->data( ind ).image );
}

void StatusModel::updateState( int ind )
{
  StatusWidget *status = static_cast<StatusWidget*>( view->indexWidget( index( ind, 0 ) ) );
  Q_ASSERT(status);
  status->setState( statusList->data( ind ).state );
}

void StatusModel::removeStatus( int ind )
{
  if ( ind == rowCount() - 1 )
    return;

  StatusWidget *status;

  for ( int i = ind; i < statusList->size() - 1; ++i ) {
    status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
    status->setStatusData( statusList->data(i + 1) );
  }
  status = static_cast<StatusWidget*>( view->indexWidget( index( statusList->size() - 1, 0 ) ) );
  Q_ASSERT(status);
  status->initialize();
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
    StatusWidget *status = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
    Q_ASSERT(status);
    StatusModel::StatusState state = statusList->state( currentIndex.row() );
    if ( state != StatusModel::STATE_UNREAD ) {
      state = StatusModel::STATE_READ;
      statusList->setState( currentIndex.row(), state );
      status->setState( StatusModel::STATE_READ );
    }
    currentIndex = QModelIndex();
  }
}

void StatusModel::setTheme( const ThemeData &theme )
{
  StatusWidget::setTheme( theme );
  StatusWidget *status;
  for ( int i = 0; i < rowCount(); i++ ) {
    status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
    Q_ASSERT(status);
    status->applyTheme();
  }
}

void StatusModel::setStatusList( StatusList *statusList )
{
  if ( this->statusList ) {
    disconnect( this->statusList, SIGNAL(statusAdded(int)), this, SLOT(updateDisplay(int)) );
    disconnect( this->statusList, SIGNAL(dataChanged(int)), this, SLOT(updateDisplay(int)) );
    disconnect( this->statusList, SIGNAL(statusDeleted(int)), this, SLOT(removeStatus(int)) );
    disconnect( this->statusList, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)) );
    disconnect( this->statusList, SIGNAL(favoriteChanged(int)), this, SLOT(updateDisplay(int)) );
    disconnect( this->statusList, SIGNAL(imageChanged(int)), this, SLOT(updateImage(int)) );
  }

  this->statusList = statusList;
  connect( this->statusList, SIGNAL(statusAdded(int)), this, SLOT(updateDisplay(int)) );
  connect( this->statusList, SIGNAL(dataChanged(int)), this, SLOT(updateDisplay(int)) );
  connect( this->statusList, SIGNAL(statusDeleted(int)), this, SLOT(removeStatus(int)) );
  connect( this->statusList, SIGNAL(stateChanged(int)), this, SLOT(updateState(int)) );
  connect( this->statusList, SIGNAL(favoriteChanged(int)), this, SLOT(updateDisplay(int)) );
  connect( this->statusList, SIGNAL(imageChanged(int)), this, SLOT(updateImage(int)) );

  // for cleaning up the list when switching to public timeline that could have
  // less statuses than requested maximum
  StatusWidget *status;
  if ( statusList->size() < maxStatusCount ) {
    for ( int i = statusList->size(); i < maxStatusCount; ++i ) {
      status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
      Q_ASSERT(status);
      status->initialize();
      item( i )->setSizeHint( status->size() );
    }
  }

  int active = statusList->active();
  if ( active == -1 ) {
    currentIndex = QModelIndex();
    view->setCurrentIndex( index( 0, 0 ) );
    view->scrollTo( index( 0, 0 ) );
  } else {
    selectStatus( index( active, 0 ) );
    view->setCurrentIndex( currentIndex );
    view->scrollTo( currentIndex );
  }

  StatusWidget::setCurrentLogin( this->statusList->login() );
  StatusWidget::setCurrentNetwork( this->statusList->network() );
  updateDisplay();
}

StatusList * StatusModel::getStatusList() const
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

void StatusModel::clear()
{
  if ( !statusList )
    return;

  statusList = 0;

  StatusWidget *status;
  for ( int i = 0; i < rowCount(); ++i )
  {
    status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
    Q_ASSERT(status);
    status->initialize();
  }
  updateDisplay();
}

void StatusModel::sendDeleteRequest( quint64 id, Entry::Type type )
{
  qDebug() << "StatusModel::sendDeleteRequest";
  emit destroy( statusList->network(), statusList->login(), id, type );
}

void StatusModel::sendFavoriteRequest( quint64 id, bool favorited )
{
  qDebug() << "StatusModel::sendFavoriteRequest";
  emit favorite( statusList->network(), statusList->login(), id, favorited );
}

void StatusModel::sendDMRequest( const QString &screenName )
{
  qDebug() << "StatusModel::sendDMRequest";
  emit postDM( statusList->network(), statusList->login(), screenName );
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

  StatusModel::StatusState state;
  StatusWidget *status;

  if ( currentIndex.isValid() ) {
    state = statusList->state( currentIndex.row() );
    if ( state != StatusModel::STATE_UNREAD ) {
      state = StatusModel::STATE_READ;
      statusList->setState( currentIndex.row(), state );

      status = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
      Q_ASSERT(status);
      status->setState( state );
    }
  }

  currentIndex = index;

  statusList->setState( index.row(), StatusModel::STATE_ACTIVE );

  status = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
  Q_ASSERT(status);
  status->setState( StatusModel::STATE_ACTIVE );

  view->setCurrentIndex( currentIndex );
}

void StatusModel::selectStatus( StatusWidget *statusWidget )
{
  if ( !statusList || statusWidget->getState() == StatusModel::STATE_DISABLED )
    return;

  Status status;
  StatusWidget *widget;
  if ( currentIndex.isValid() && currentIndex.row() < statusList->size() ) {
    status = statusList->data( currentIndex.row() );
    if ( status.state != StatusModel::STATE_UNREAD ) {
      statusList->setState( currentIndex.row(), StatusModel::STATE_READ );
      widget = static_cast<StatusWidget*>( view->indexWidget( currentIndex ) );
      Q_ASSERT(widget);
      widget->setState( StatusModel::STATE_READ );
    }
  }
  for ( int i = 0; i < statusList->size(); i++ ) {
    status = statusList->data( i );
    if ( status.entry.id == statusWidget->getId() ) {
      currentIndex = item(i)->index();
      statusList->setState( currentIndex.row(), StatusModel::STATE_ACTIVE );
      statusWidget->setState( StatusModel::STATE_ACTIVE );
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
        status.state = StatusModel::STATE_ACTIVE;
      else
        status.state = StatusModel::STATE_READ;
      statusList->setState(i, status.state );
    }
  }
}

void StatusModel::retranslateUi()
{
  // TODO :)
//  return;
  StatusWidget *status;
  for ( int i = 0; i < rowCount(); i++ ) {
    status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
    status->retranslateUi();
  }
}

void StatusModel::resizeData( int width, int oldWidth )
{
  Q_UNUSED(oldWidth);

  StatusWidget *status;
  for ( int i = 0; i < rowCount(); i++ ) {
    status = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
    status->resize( width, status->size().height() );
    item(i)->setSizeHint( status->size() );
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

void StatusModel::setImageForUrl( const QString& url, QPixmap *image )
{
  Status status;
  StatusWidget *statusWidget;
  for ( int i = 0; i < statusList->size(); i++ ) {
    status = statusList->data(i);
    if ( url == status.entry.userInfo.imageUrl ) {
      status.image = *image;
      statusWidget = static_cast<StatusWidget*>( view->indexWidget( index( i, 0 ) ) );
      statusWidget->setImage( *image );
    }
  }
}

void StatusModel::emitOpenBrowser( QString address )
{
  emit openBrowser( QUrl( address ) );
}


/*! \class StatusModel
  \brief A class for managing the StatusWidget list content.

  This class contains a model for a list view displaying status updates.
  It is responsible for behind the scenes management of all the actions
  like adding, sorting, deleting and updating Statuses. Furthermore since
  it is directly connected with all the Statuses, it manages their selecting
  and deselecting.
*/

/*! \fn StatusModel::StatusModel( int margin, StatusListView *parentListView, QObject *parent = 0 )
    Creates a status list model with a given \a parent.
    \param margin Holds the width of the StatusWidget list's scrollbar useful when setting
                  size of the StatusWidget widgets.
    \param parentListView The list view that the model serves for.
    \param parent A parent for the status list model.
*/

/*! \fn void StatusModel::deselectCurrentIndex();
    Removes selection from currently highlighted item.
    \sa selectStatus()
*/

/*! \fn void StatusModel::setTheme( const ThemeData &theme )
    Sets \a theme to be the current theme for all the Statuses.
    \param theme The theme to be set.
*/

/*! \fn void StatusModel::setMaxStatusCount( int count )
    Sets maximum amount of Statuses on a list.
    \param count The given maximum Statuses amount.
*/

/*! \fn void StatusModel::insertStatus( Entry *entry )
    Creates a StatusWidget class instance as a representation of \a entry and adds it
    to the list in an appropriate place (sorting chronogically).
    \param entry The entry on which the new StatusWidget bases.
    \sa deleteStatus()
*/

/*! \fn void StatusModel::deleteStatus( int id )
    Removes StatusWidget of the given id from the model and deletes it.
    \param id An id of the StatusWidget to be deleted.
    \sa insertStatus()
*/

/*! \fn void StatusModel::slotDirectMessagesChanged( bool isEnabled )
    Removes all direct messages from the model and deletes them. Used when
    User disables direct messages download.
*/

/*! \fn void StatusModel::selectStatus( const QModelIndex &index )
    Highlights a StatusWidget of a given \a index. Used by a mouse press event on the
    StatusWidget list view.
    \param index The model index of the element to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void StatusModel::selectStatus( StatusWidget *status )
    Highlights a given StatusWidget. Used by a mouse press event on the StatusWidget's
    internal status display widget.
    \param status A StatusWidget to be selected.
    \sa deselectCurrentIndex()
*/

/*! \fn void StatusModel::markAllAsRead()
    Sets all Statuses' state to StatusModel::STATE_READ.
*/

/*! \fn void StatusModel::sendNewsInfo()
    Counts unread Statuses and messages and emits newStatuses() signal
    to notify MainWindow about a tray icon message to pop up.
*/

/*! \fn void StatusModel::retranslateUi()
    Retranslates all Statuses.
*/

/*! \fn void StatusModel::resizeData( int width, int oldWidth )
    Resizes Statuses according to given values.
    \param width New width of MainWindow.
    \param oldWidth Old width of MainWindow.
*/

/*! \fn void StatusModel::moveFocus( bool up )
    Selects the current StatusWidget's neighbour, according to the given \a up parameter.
    \param up Selects upper StatusWidget if true, otherwise selects lower one.
    \sa selectStatus()
    \sa deselectCurrentIndex()
*/

/*! \fn void StatusModel::setImageForUrl( const QString& url, QPixmap image )
    Assigns the given \a image to all the Statuses having \a url as their profile image URL.
    \param url Profile image URL of the StatusWidget.
    \param image Image to be set for the StatusWidget(s).
*/

/*! \fn void StatusModel::setModelToBeCleared( bool wantsPublic, bool userChanged )
    Evaluates an internal flag that indicates if a model has to be completely
    cleared before inserting new Statuses. This occurs e.g. when switching from
    public to friends timeline, or when synchronising with friends timeline
    and changing authenticating user.
    \param wantsPublic Indicates if public timeline will be requested
                       upon next update.
    \param userChanged Indicates if the authentcating user has changed since the
                       last update.
*/

/*! \fn void StatusModel::setPublicTimelineRequested( bool b )
    Sets the flag indicating if the public timeline will be requested upon the
    next connection.
    \param b The new flag's value.
*/

/*! \fn void StatusModel::restatus( QString message )
    Passes the restatus message from a particular StatusWidget to the MainWindow.
    \param message Restatus status message.
    \sa reply()
*/

/*! \fn void StatusModel::destroy( int id )
    A request to destroy a StatusWidget, passed from a specific StatusWidget to the Core class instance.
    \param id The StatusWidget's id.
*/

/*! \fn void StatusModel::newStatuses( int statusesCount, QStringList statusesNames, int messagesCount, QStringList messagesNames )
    Emitted by \ref sendNewsInfo() to notify MainWindow of a new statuses.
    \param statusesCount Amount of the new statuses.
    \param statusesNames List of new statuses authors.
    \param messagesCount Amount of the new direct messages.
    \param messagesNames List of new direct messages authors.
*/

/*! \fn void StatusModel::openBrowser( QUrl address )
    Emitted to pass the request to open web browser to the Core class instance.
    \param address Web address to be accessed.
*/

/*! \fn void StatusModel::reply( const QString &name, quint64 inReplyTo )
    Passes the reply request from a particular StatusWidget to the MainWindow.
    \param name Login of the original message author.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa restatus()
*/

/*! \fn void StatusModel::about()
    Passes the request to popup an about dialog to the MainWindow.
*/
