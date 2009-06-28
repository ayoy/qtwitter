/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#include <QDebug>
#include <QClipboard>
#include <QMenu>
#include <QSignalMapper>
#include <twitterapi/twitterapi_global.h>
#include "statuswidget.h"
#include "settings.h"
#include "statuslist.h"
#include "ui_statuswidget.h"

int StatusWidget::scrollBarWidth = 0;
int StatusWidget::currentWidth = 0;
ThemeData StatusWidget::currentTheme = ThemeData();
TwitterAPI::SocialNetwork StatusWidget::currentNetwork = TwitterAPI::SOCIALNETWORK_TWITTER;
QString StatusWidget::currentLogin = QString();
StatusWidget* StatusWidget::activeStatus = 0;

StatusWidget::StatusWidget( StatusModel *parentModel, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  statusState( StatusModel::STATE_DISABLED ),
  statusData(0),
  statusListModel( parentModel ),
  m_ui(new Ui::StatusWidget)
{
  m_ui->setupUi( this );
  m_ui->favoriteReplyButton->hide();
  m_ui->infoButton->hide();
  m_ui->replyDeleteButton->hide();

  m_ui->favoriteReplyButton->setToolTip( tr( "Add to Favorites" ) );

  QFont timeStampFont = m_ui->timeStamp->font();
#ifdef Q_WS_MAC
  timeStampFont.setPointSize( timeStampFont.pointSize() - 2 );
#else
  timeStampFont.setPointSize( timeStampFont.pointSize() - 1 );
#endif
  m_ui->timeStamp->setFont( timeStampFont );

  connect( m_ui->replyDeleteButton, SIGNAL(clicked()), this, SLOT(handleReplyDeleteButton()));
  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(StatusWidget*)), statusListModel, SLOT(selectStatus(StatusWidget*)) );

  applyTheme();
  m_ui->userName->setText( "" );
  m_ui->userStatus->setHtml( "" );
  m_ui->timeStamp->setText( "" );

  resize( currentWidth, height() );
  adjustSize();
  createMenu();
  m_ui->userStatus->setMenu( menu );
}

StatusWidget::~StatusWidget()
{
  delete m_ui;
}

void StatusWidget::createMenu()
{
  menu = new QMenu( this );
  signalMapper = new QSignalMapper( this );

  replyAction = new QAction( this );
  replyAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_R ) );
  menu->addAction( replyAction );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(slotReply()) );
  connect( this, SIGNAL(reply(QString,quint64)), statusListModel, SIGNAL(reply(QString,quint64)) );

  retweetAction = new QAction( tr( "Retweet" ), this );
  retweetAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ) );
  menu->addAction( retweetAction );
  connect( retweetAction, SIGNAL(triggered()), this, SLOT(slotRetweet()) );
  connect( this, SIGNAL(retweet(QString)), statusListModel, SIGNAL(retweet(QString)) );

  dmAction = new QAction( this );
  dmAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_D ) );
  menu->addAction( dmAction );
  connect( dmAction, SIGNAL(triggered()), this, SLOT(slotDM()) );

  menu->addSeparator();

  favoriteAction = new QAction( tr( "Add to Favorites" ), this );
  favoriteAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_F ) );
  menu->addAction( favoriteAction );
  connect( favoriteAction, SIGNAL(triggered()), this, SLOT(slotFavorite()) );

  copylinkAction = new QAction( tr( "Copy link to this status" ), this );
  copylinkAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
  menu->addAction( copylinkAction );
  connect( copylinkAction, SIGNAL(triggered()), this, SLOT(slotCopyLink()) );

  deleteAction = new QAction( this );
  deleteAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace ) );
  menu->addAction( deleteAction );
  connect( deleteAction, SIGNAL(triggered()), this, SLOT(slotDelete()) );

  markallasreadAction = new QAction( tr( "Mark list as read" ), this );
  markallasreadAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ) );
  menu->addAction( markallasreadAction );
  connect( markallasreadAction, SIGNAL(triggered()), statusListModel, SLOT(markAllAsRead()) );

  markeverythingasreadAction = new QAction( tr( "Mark everything as read" ), this );
  markeverythingasreadAction->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_A ) );
  menu->addAction( markeverythingasreadAction );
  connect( markeverythingasreadAction, SIGNAL(triggered()), statusListModel, SIGNAL(markEverythingAsRead()) );

  menu->addSeparator();

  gototwitterpageAction = new QAction( this );
  gototwitterpageAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_T ) );
  menu->addAction( gototwitterpageAction );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), statusListModel, SLOT(emitOpenBrowser(QString)) );

  gotohomepageAction = new QAction( tr( "Go to User's homepage" ), this);
  gotohomepageAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_H ) );
  menu->addAction( gotohomepageAction );
  connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
}

void StatusWidget::setupMenu()
{
  if ( !menu || !signalMapper )
    return;

  replyAction->setText( tr("Reply to %1" ).arg( statusData->userInfo.screenName ) );
  // TODO: enable replying when at least one account is configured
  if ( statusData->type != Entry::Status || currentLogin == TwitterAPI::PUBLIC_TIMELINE ) {
    replyAction->setEnabled( false );
  } else {
    replyAction->setEnabled( true );
  }

  // TODO: enable retweeting when at least one account is configured
  if ( statusData->type != Entry::Status || currentLogin == TwitterAPI::PUBLIC_TIMELINE ) {
    retweetAction->setEnabled( false );
  } else {
    retweetAction->setEnabled( true );
  }

  dmAction->setText( tr( "Direct message %1" ).arg( statusData->userInfo.screenName ) );
  dmAction->setEnabled( !(currentLogin == TwitterAPI::PUBLIC_TIMELINE) && !statusData->isOwn );

  menu->addSeparator();

  if ( statusData->type != Entry::Status ) {
    copylinkAction->setEnabled( false );
  } else {
    copylinkAction->setEnabled( true );
  }

  if ( statusData->isOwn || statusData->type == Entry::DirectMessage ) {
    deleteAction->setEnabled( true );
  } else {
    deleteAction->setEnabled( false );
  }

  menu->addSeparator();

  signalMapper->removeMappings( gototwitterpageAction );
  if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    gototwitterpageAction->setText( tr( "Go to User's Identi.ca page" ) );
    signalMapper->setMapping( gototwitterpageAction, "http://identi.ca/" + statusData->userInfo.screenName );
  } else {
    gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
    signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + statusData->userInfo.screenName );
  }

  signalMapper->removeMappings( gotohomepageAction );
  if ( !statusData->userInfo.homepage.compare("") ) {
    gotohomepageAction->setEnabled( false );
  } else {
    gotohomepageAction->setEnabled( true );
    signalMapper->setMapping( gotohomepageAction, statusData->userInfo.homepage );
  }
}

void StatusWidget::resize( const QSize &s )
{
  m_ui->frame->resize( s );
  QWidget::resize( s );
}

void StatusWidget::resize( int w, int h )
{
  w -= StatusWidget::scrollBarWidth;
  QWidget::resize( w, h );
  m_ui->frame->resize( w, h );
  m_ui->userStatus->resize( size().width() - m_ui->userStatus->geometry().x() - 6, m_ui->userStatus->size().height() );
  m_ui->userName->resize( m_ui->userStatus->width(), m_ui->userName->height() );
  m_ui->timeStamp->resize( m_ui->userStatus->width(), m_ui->timeStamp->height() );
  adjustSize();
}

const Entry& StatusWidget::data() const
{
  return *statusData;
}

void StatusWidget::initialize()
{
  statusData = 0;

  m_ui->userName->clear();
  m_ui->userStatus->clear();
  m_ui->userImage->clear();
  m_ui->timeStamp->clear();

  setState( StatusModel::STATE_DISABLED );
  adjustSize();
}

void StatusWidget::setStatusData( const Status &status )
{
  statusData = &status.entry;
  retranslateUi();
  m_ui->favoriteReplyButton->disconnect();

  m_ui->userName->setText( statusData->userInfo.name );
  m_ui->userStatus->setText( statusData->text );
  m_ui->userImage->setPixmap( status.image );


  //adjust tooltip for reply/delete button
  if ( statusData->type == Entry::Status ) {
    if ( statusData->isOwn ) {
      m_ui->replyDeleteButton->setIcon( QIcon(":/icons/cross_16.png") );
      m_ui->replyDeleteButton->setToolTip( tr( "Delete status" ) );
    } else {
      m_ui->replyDeleteButton->setIcon( QIcon(":/icons/reply_16.png") );
      m_ui->replyDeleteButton->setToolTip( tr( "Reply to %1" ).arg( statusData->userInfo.screenName ) );
    }
  } else {
    m_ui->replyDeleteButton->setIcon( QIcon(":/icons/cross_16.png") );
    m_ui->replyDeleteButton->setToolTip( tr( "Delete message" ) );
  }

  if ( statusData->type == Entry::Status )
    deleteAction->setText( tr( "Delete status" ) );
  else
    deleteAction->setText( tr( "Delete message" ) );

  if ( currentLogin != TwitterAPI::PUBLIC_TIMELINE ) {

    if ( statusData->type == Entry::DirectMessage ) {

      m_ui->favoriteReplyButton->setIcon( QIcon( ":/icons/reply_16.png" ) );
      m_ui->favoriteReplyButton->setToolTip( tr( "Reply to %1" ).arg( statusData->userInfo.screenName ) );
      connect( m_ui->favoriteReplyButton, SIGNAL(clicked()), this, SLOT(slotDM()) );

    } else {

      if ( statusData->favorited ) {
        m_ui->favoriteReplyButton->setIcon( QIcon( ":/icons/star_on_16.png" ) );
        if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
          m_ui->favoriteReplyButton->setToolTip( QString() );
          favoriteAction->setText( tr( "Remove from Favorites" ) );
          favoriteAction->setEnabled( false );
        } else {
          m_ui->favoriteReplyButton->setToolTip( tr( "Remove from Favorites" ) );
          favoriteAction->setText( m_ui->favoriteReplyButton->toolTip() );
          favoriteAction->setEnabled( true );
        }
      } else {
        m_ui->favoriteReplyButton->setIcon( QIcon( ":/icons/star_off_16.png" ) );
        m_ui->favoriteReplyButton->setToolTip( tr( "Add to Favorites" ) );
        favoriteAction->setText( m_ui->favoriteReplyButton->toolTip() );
        favoriteAction->setEnabled( true );
      }
      connect( m_ui->favoriteReplyButton, SIGNAL(clicked()), this, SLOT(slotFavorite()) );

    }

  } else {
    favoriteAction->setEnabled( false );
  }

  //display status's send time
  if( statusData->localTime.date() >= QDateTime::currentDateTime().date()) //today
    m_ui->timeStamp->setText( statusData->localTime.time().toString(Qt::SystemLocaleShortDate) );
  else  //yesterday or earlier
    m_ui->timeStamp->setText( statusData->localTime.toString(Qt::SystemLocaleShortDate) );

  //display in_reply_to link
  if( statusData->hasInReplyToStatusId ) {
    QString inReplyToUrl;
    if ( this->currentNetwork == TwitterAPI::SOCIALNETWORK_TWITTER ) {
      inReplyToUrl = "http://twitter.com/" + statusData->inReplyToScreenName + "/statuses/" + QString::number( statusData->inReplyToStatusId );
    }
    else if ( this->currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA )
      //for identica it works as is
      inReplyToUrl = "http://identi.ca/notice/" + QString::number( statusData->inReplyToStatusId );

    m_ui->timeStamp->setText( m_ui->timeStamp->text().append( " " )
                              .append( tr( "in reply to %1" )
                                       // TODO: links theming
                                       .arg( QString( "<a style=\"color:rgb(255, 248, 140)\" href=%1>%2</a>" )
                                             .arg( inReplyToUrl, statusData->inReplyToScreenName ) ) ) );
  }

  m_ui->infoButton->setData( &status );


  setState( status.state );
  setupMenu();
  adjustSize();
}

void StatusWidget::setImage( const QPixmap &pixmap )
{
  m_ui->userImage->setPixmap( pixmap );
}

void StatusWidget::setState( StatusModel::StatusState state )
{
  statusState = state;
  applyTheme();
}

StatusModel::StatusState StatusWidget::getState() const
{
  return statusState;
}

ThemeData StatusWidget::getTheme()
{
  return currentTheme;
}

void StatusWidget::setTheme( const ThemeData &theme )
{
  currentTheme = theme;
}

void StatusWidget::applyTheme()
{
  bool inreply = false;
  QString inReplyString;
  QString linkColor;
  QRegExp rx( "rgb\\(([\\d, ]+)\\)", Qt::CaseInsensitive );
  rx.setMinimal( true );

  if ( statusData ) {
    if ( (inreply = statusData->hasInReplyToStatusId) ) {
      inReplyString = m_ui->timeStamp->text();
      inReplyString.replace( rx, "rgb(%1)" );
    }
  }

  ThemeElement newState;

  switch ( statusState ) {
  case StatusModel::STATE_UNREAD:
    newState = currentTheme.unread;
    break;
  case StatusModel::STATE_ACTIVE:
    newState = currentTheme.active;
    break;
  case StatusModel::STATE_READ:
    newState = currentTheme.read;
    break;
  case StatusModel::STATE_DISABLED:
    newState = currentTheme.disabled;
    break;
  }

  if ( inreply && rx.indexIn( newState.linkColor ) != -1 ) {
      linkColor = rx.cap(1);
      m_ui->timeStamp->setText( inReplyString.arg( linkColor ) );
  }

  setStyleSheet( newState.styleSheet );
  m_ui->userStatus->document()->setDefaultStyleSheet( newState.linkColor );

  // this is strange, but style sheet will work only after resetting userStatus
  if ( statusData )
    m_ui->userStatus->setText( statusData->text );
}

void StatusWidget::retranslateUi()
{
  if ( statusData ) {
    dmAction->setText( tr( "Direct message %1" ).arg( statusData->userInfo.screenName ) );
    dmAction->setEnabled( !(currentLogin == TwitterAPI::PUBLIC_TIMELINE) && !statusData->isOwn );

    if ( statusData->favorited ) {
      if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
        m_ui->favoriteReplyButton->setToolTip( QString() );
        favoriteAction->setText( tr( "Remove from Favorites" ) );
        favoriteAction->setEnabled( false );
      } else {
        m_ui->favoriteReplyButton->setToolTip( tr( "Remove from Favorites" ) );
        favoriteAction->setText( m_ui->favoriteReplyButton->toolTip() );
        favoriteAction->setEnabled( true );
      }
    } else {
      m_ui->favoriteReplyButton->setToolTip( tr( "Add to Favorites" ) );
    }

    m_ui->infoButton->setToolTip( tr( "About %1" ).arg( statusData->userInfo.screenName ) );
    replyAction->setText( tr( "Reply to %1" ).arg( statusData->userInfo.screenName ) );

    if ( statusData->type == Entry::Status )
      deleteAction->setText( tr( "Delete status" ) );
    else
      deleteAction->setText( tr( "Delete message" ) );

    if ( statusData->isOwn )
      m_ui->replyDeleteButton->setToolTip( deleteAction->text() );
    else
      m_ui->replyDeleteButton->setToolTip( replyAction->text() );

    retweetAction->setText( tr( "Retweet" ) );
    copylinkAction->setText( tr( "Copy link to this status" ) );
    deleteAction->setText( tr( "Delete status" ) );
    markallasreadAction->setText( tr( "Mark list as read" ) );
    markeverythingasreadAction->setText( tr( "Mark everything as read" ) );
    gotohomepageAction->setText( tr( "Go to User's homepage" ) );

    if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
      gototwitterpageAction->setText( tr( "Go to User's Identi.ca page" ) );
    } else {
      gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
    }
  }
}

quint64 StatusWidget::getId() const
{
  return statusData->id;
}

// static
void StatusWidget::setScrollBarWidth( int width )
{
  scrollBarWidth = width;
}
void StatusWidget::setCurrentWidth( int width )
{
  currentWidth = width;
}
void StatusWidget::setCurrentLogin( const QString &login )
{
  currentLogin = login;
}
void StatusWidget::setCurrentNetwork( TwitterAPI::SocialNetwork network )
{
  currentNetwork = network;
}
// static_end

// TODO: magic numbers!!!!!!!!
void StatusWidget::adjustSize()
{
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->frame->resize( m_ui->frame->width(), qMax(82, m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + m_ui->timeStamp->height() + 8) );
  resize( m_ui->frame->size() );
  m_ui->timeStamp->move( m_ui->timeStamp->x(), height() - m_ui->timeStamp->height() - 6 );//m_ui->userStatus->geometry().y() + m_ui->userStatus->height() );
}

void StatusWidget::slotReply()
{
  emit reply( statusData->userInfo.screenName, statusData->id );
}

void StatusWidget::slotRetweet()
{
  emit retweet( QString("RT @" + statusData->userInfo.screenName + ": " + statusData->originalText ) );
}

void StatusWidget::slotDM()
{
  statusListModel->sendDMRequest( statusData->userInfo.screenName );
}

void StatusWidget::slotCopyLink()
{
  if ( currentNetwork == TwitterAPI::SOCIALNETWORK_TWITTER )
    QApplication::clipboard()->setText( "http://twitter.com/" + statusData->userInfo.screenName + "/statuses/" + QString::number( statusData->id ) );
  else if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA )
    QApplication::clipboard()->setText( "http://identi.ca/notice/" + QString::number( statusData->id ) );
}

void StatusWidget::slotDelete()
{
  statusListModel->sendDeleteRequest( statusData->id, statusData->type );
}

void StatusWidget::slotFavorite()
{
  //if a status already is favorited, send a request to disfavor it
  bool setFavorited = !statusData->favorited;

  if ( !setFavorited && currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA )
    return;

  statusListModel->sendFavoriteRequest( statusData->id, setFavorited );
}

void StatusWidget::changeEvent( QEvent *e )
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:;
  }
}

void StatusWidget::enterEvent( QEvent *e )
{
  if ( statusState != StatusModel::STATE_DISABLED ) {
    if ( currentLogin != TwitterAPI::PUBLIC_TIMELINE ) {
      m_ui->replyDeleteButton->show();
      m_ui->favoriteReplyButton->show();
    }
    m_ui->infoButton->show();
    e->accept();
  }
}

void StatusWidget::leaveEvent( QEvent *e )
{
  m_ui->favoriteReplyButton->hide();
  m_ui->replyDeleteButton->hide();
  m_ui->infoButton->hide();
  QWidget::leaveEvent( e );
}

void StatusWidget::mousePressEvent( QMouseEvent *e )
{
  emit focusRequest();
  if ( e->button() == Qt::RightButton && statusData ) {
    menu->exec( QCursor::pos() );
  }
}

void StatusWidget::focusRequest()
{
  emit selectMe( this );
}

void StatusWidget::handleReplyDeleteButton()
{
  if ( statusData->isOwn || statusData->type == Entry::DirectMessage )
    slotDelete();
  else
    slotReply();
}

/*! \class StatusWidget
    \brief A widget representation of an Entry class.

    This widget class contains the status data displayed in a custom widget that is put
    on the status list view.
*/

/*! \enum StatusWidget::State
    \brief A StatusWidget's state.

    Used to specify the StatusWidget's current state, based on the User's selection.
*/

/*! \var StatusWidget::State StatusWidget::Unread
    The StatusWidget is unread.
*/

/*! \var StatusWidget::State StatusWidget::Read
    The StatusWidget is read.
*/

/*! \var StatusWidget::State StatusWidget::Active
    The StatusWidget is active, i.e. currently highlighted.
*/

/*! \fn explicit StatusWidget::StatusWidget( const Entry &entry, const QPixmap &image, QWidget *parent = 0 )
    Creates a new StatusWidget with a given \a parent, fills its data with the given
    \a entry and sets its user image to \a image.
*/

/*! \fn virtual StatusWidget::~StatusWidget()
    A virtual destructor.
*/

/*! \fn void StatusWidget::resize( const QSize& size )
    Resizes a StatusWidget to the given \a size.
    \param size New size of the StatusWidget.
    \sa adjustSize()
*/

/*! \fn void StatusWidget::resize( int w, int h )
    Resizes a StatusWidget to the width given by \a w and height given by \a h.
    Invokes adjustSize() to additionally correct the height of the StatusWidget.
    \param w New width of the StatusWidget.
    \param h New height of the StatusWidget.
    \sa adjustSize()
*/

/*! \fn void StatusWidget::setIcon( const QPixmap &image )
    Sets the StatusWidget's User profile image to \a image.
    \param image An image to be set.
*/

/*! \fn void StatusWidget::applyTheme()
    Applies a theme to the StatusWidget, according to its current state.
    \sa setState(), getState()
*/

/*! \fn void StatusWidget::retranslateUi()
    Retranslates all the translatable GUI elements of the StatusWidget.
    Used when changing UI language on the fly.
*/

/*! \fn bool StatusWidget::isRead() const
    Used to figure out if the StatusWidget is already read.
    \returns True when StatusWidget's state is \ref Read or \ref Active, false when StatusWidget's
             state is \ref Unread.
    \sa getState()
*/

/*! \fn StatusWidget::State StatusWidget::getState() const
    Used to figure out the actual state of the StatusWidget.
    \returns The current state of the StatusWidget.
    \sa setState(), isRead()
*/

/*! \fn void StatusWidget::setState( StatusWidget::State state );
    Sets StatusWidget's current state to \a state.
    \param state A \ref State to set for the StatusWidget.
*/

/*! \fn static ThemeData StatusWidget::getTheme();
    Provides information about the theme that is currently set to all the Statuses.
    \returns Current Statuses' theme.
    \sa setTheme()
*/

/*! \fn static void StatusWidget::setTheme( const ThemeData &theme )
    Sets the current theme for all the Statuses.
    \param theme A theme to be set.
    \sa getTheme()
*/

/*! \fn static void StatusWidget::setStatusListModel( StatusModel *statusModel )
    Sets a StatusWidget list model. The model is used by a StatusWidget to receive signals
    from a StatusWidget menu.
    \param statusModel The model to be set.
*/

/*! \fn void StatusWidget:adjustSize()
    Adjusts height of the StatusWidget widget to fit the whole status.
    \sa resize()
*/

/*! \fn void StatusWidget::menuRequested()
    Opens a StatusWidget menu.
*/

/*! \fn void StatusWidget::slotReply()
    Prepares and emits a \ref reply() signal.
    \sa reply()
*/

/*! \fn void StatusWidget::slotRetweet()
    Prepares a retweet (citation of other user's status) and emits a
    \ref retweet() signal to post it.
    \sa retweet()
*/

/*! \fn void StatusWidget::slotCopyLink()
    Copies a link to the StatusWidget to system clipboard.
*/

/*! \fn void StatusWidget::reply( const QString &name, quint64 inReplyTo )
    Emitted to notify the MainWindow class instance about the User's request
    to send a reply.
    \param name Login of the User to whom a reply is addressed.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa slotReply()
*/

/*! \fn void StatusWidget::retweet( QString message )
    Emitted to notify the Core class instance about the User's request
    to retweet a status.
    \param message A retweet status message prepared by \ref slotRetweet().
    \sa slotRetweet()
*/

/*! \fn void StatusWidget::markAllAsRead()
    Emitted to notify the StatusModel class instance about User's request
    to mark all the Statuses in a list as read.
*/

/*! \fn void StatusWidget::selectMe( StatusWidget *statuswidget )
    Emitted to notify the StatusModel class instance about User's request
    to select (highlight) the current StatusWidget.
    \param status A StatusWidget to be highlighted. Usually \a this.
*/

/*! \fn void StatusWidget::changeEvent( QEvent *e )
    Reimplemented to force \ref retranslateUi() upon a language change.
    \param e A QEvent event's representation.
    \sa retranslateUi()
*/

/*! \fn void StatusWidget::enterEvent( QEvent *e )
    Reimplemented to show the menu icon when hovered by mouse.
    \param e A QEvent event's representation.
*/

/*! \fn void StatusWidget::leaveEvent( QEvent *e )
    Reimplemented to hide the menu icon when mouse leaves the StatusWidget.
    \param e A QEvent event's representation.
*/
