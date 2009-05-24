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
#include "tweet.h"
#include "ui_tweet.h"
#include "settings.h"
#include "statuslist.h"

int Tweet::scrollBarWidth = 0;
int Tweet::currentWidth = 0;
ThemeData Tweet::currentTheme = ThemeData();
TwitterAPI::SocialNetwork Tweet::currentNetwork = TwitterAPI::SOCIALNETWORK_TWITTER;
QString Tweet::currentLogin = QString();

Tweet::Tweet( TweetModel *parentModel, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  tweetState( TweetModel::STATE_DISABLED ),
  tweetData(0),
  tweetListModel( parentModel ),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi( this );

  QFont timeStampFont = m_ui->timeStamp->font();
  timeStampFont.setPointSize( timeStampFont.pointSize() - 1 );
  m_ui->timeStamp->setFont( timeStampFont );

  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(Tweet*)), tweetListModel, SLOT(selectTweet(Tweet*)) );
  connect( m_ui->replyButton, SIGNAL(clicked()), this, SLOT(slotReply()));

  applyTheme();
  m_ui->userName->setText( "" );
  m_ui->userStatus->setHtml( "" );
  m_ui->timeStamp->setText( "" );

  resize( currentWidth, height() );
  adjustSize();
  setFocusProxy( m_ui->userStatus );
  createMenu();
  m_ui->userStatus->setMenu( menu );
}

Tweet::Tweet( Entry *entry, TweetModel::TweetState *state, const QPixmap &image, TweetModel *parentModel, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  tweetState( *state ),
  tweetData( entry ),
  tweetListModel( parentModel ),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi( this );

  QFont timeStampFont = m_ui->timeStamp->font();
  timeStampFont.setPointSize( timeStampFont.pointSize() - 1 );
  m_ui->timeStamp->setFont( timeStampFont );

  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(Tweet*)), tweetListModel, SLOT(selectTweet(Tweet*)) );
  connect( m_ui->replyButton, SIGNAL(clicked()), this, SLOT(slotReply()));

  applyTheme();
  m_ui->userName->setText( tweetData->name );
  m_ui->userStatus->setHtml( tweetData->text );

  m_ui->userImage->setPixmap( image );
  adjustSize();
  setFocusProxy( m_ui->userStatus );
  createMenu();
  m_ui->userStatus->setMenu( menu );
}

Tweet::~Tweet()
{
  delete m_ui;
}

void Tweet::createMenu()
{
  menu = new QMenu( this );
  signalMapper = new QSignalMapper( this );

  replyAction = new QAction( this );
  replyAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_R ) );
  menu->addAction( replyAction );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(slotReply()) );
  connect( this, SIGNAL(reply(QString,int)), tweetListModel, SIGNAL(reply(QString,int)) );


  retweetAction = new QAction( tr( "Retweet" ), this );
  retweetAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ) );
  menu->addAction( retweetAction );
  connect( retweetAction, SIGNAL(triggered()), this, SLOT(slotRetweet()) );
  connect( this, SIGNAL(retweet(QString)), tweetListModel, SIGNAL(retweet(QString)) );

  menu->addSeparator();

  copylinkAction = new QAction( tr( "Copy link to this tweet" ), this );
  copylinkAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ) );
  menu->addAction( copylinkAction );
  connect( copylinkAction, SIGNAL(triggered()), this, SLOT(slotCopyLink()) );

  deleteAction = new QAction( tr( "Delete tweet" ), this );
  deleteAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace ) );
  menu->addAction( deleteAction );
  connect( deleteAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( m_ui->menuButton, SIGNAL(clicked()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(int)), tweetListModel, SLOT(sendDeleteRequest(int)) );

  markallasreadAction = new QAction( tr( "Mark all as read" ), this );
  markallasreadAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ) );
  menu->addAction( markallasreadAction );
  connect( markallasreadAction, SIGNAL(triggered()), tweetListModel, SLOT(markAllAsRead()) );

  menu->addSeparator();

  gototwitterpageAction = new QAction( this );
  gototwitterpageAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_T ) );
  menu->addAction( gototwitterpageAction );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), tweetListModel, SLOT(emitOpenBrowser(QString)) );

  gotohomepageAction = new QAction( tr( "Go to User's homepage" ), this);
  gotohomepageAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_H ) );
  menu->addAction( gotohomepageAction );
  connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
}

void Tweet::setupMenu()
{
  if ( !menu || !signalMapper )
    return;

  replyAction->setText( tr("Reply to %1" ).arg( tweetData->login ) );
  // TODO: enable replying when at least one account is configured
  if ( tweetData->type != Entry::Status || currentLogin == TwitterAPI::PUBLIC_TIMELINE ) {
    replyAction->setEnabled( false );
  } else {
    replyAction->setEnabled( true );
  }

  // TODO: enable retweeting when at least one account is configured
  if ( tweetData->type != Entry::Status || currentLogin == TwitterAPI::PUBLIC_TIMELINE ) {
    retweetAction->setEnabled( false );
  } else {
    retweetAction->setEnabled( true );
  }

  menu->addSeparator();

  if ( tweetData->type != Entry::Status ) {
    copylinkAction->setEnabled( false );
  } else {
    copylinkAction->setEnabled( true );
  }

  signalMapper->removeMappings( deleteAction );
  if ( !tweetData->isOwn ) {
    deleteAction->setEnabled( false );
  } else {
    deleteAction->setEnabled( true );
    signalMapper->setMapping( deleteAction, tweetData->id );
    signalMapper->setMapping( m_ui->menuButton, tweetData->id );
  }

  menu->addSeparator();

  signalMapper->removeMappings( gototwitterpageAction );
  if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    gototwitterpageAction->setText( tr( "Go to User's Identi.ca page" ) );
    signalMapper->setMapping( gototwitterpageAction, "http://identi.ca/" + tweetData->login );
  } else {
    gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
    signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + tweetData->login );
  }

  signalMapper->removeMappings( gotohomepageAction );
  if ( !tweetData->homepage.compare("") ) {
    gotohomepageAction->setEnabled( false );
  } else {
    gotohomepageAction->setEnabled( true );
    signalMapper->setMapping( gotohomepageAction, tweetData->homepage );
  }
}

void Tweet::resize( const QSize &s )
{
  m_ui->frame->resize( s );
  QWidget::resize( s );
}

void Tweet::resize( int w, int h )
{
  w -= Tweet::scrollBarWidth;
  QWidget::resize( w, h );
  m_ui->frame->resize( w, h );
  m_ui->userStatus->resize( size().width() - m_ui->userStatus->geometry().x() - 12, m_ui->userStatus->size().height() );
  m_ui->userName->resize( m_ui->userStatus->width(), m_ui->userName->height() );
  m_ui->timeStamp->resize( m_ui->userStatus->width(), m_ui->timeStamp->height() );
  adjustSize();
}

const Entry& Tweet::data() const
{
  return *tweetData;
}

void Tweet::initialize()
{
  tweetData = 0;

  m_ui->userName->clear();
  m_ui->userStatus->clear();
  m_ui->userImage->clear();
  m_ui->timeStamp->clear();

  setState( TweetModel::STATE_DISABLED );
  adjustSize();
}

void Tweet::setTweetData( const Status &status )
{
  tweetData = &status.entry;

  m_ui->userName->setText( tweetData->name );
  m_ui->userStatus->setText( tweetData->text );
  m_ui->userImage->setPixmap( status.image );

  //display tweet's send time
  if( tweetData->localTime.date() >= QDateTime::currentDateTime().date()) //today
    m_ui->timeStamp->setText( tweetData->localTime.time().toString(Qt::SystemLocaleShortDate) );
  else  //yesterday or earlier
    m_ui->timeStamp->setText( tweetData->localTime.toString(Qt::SystemLocaleShortDate) );

  //display in_reply_to link
  if( tweetData->hasInReplyToStatusId) {
    QString inReplyToUrl;
    if ( this->currentNetwork == TwitterAPI::SOCIALNETWORK_TWITTER ) {
      inReplyToUrl = "http://twitter.com/" + tweetData->inReplyToScreenName + "/statuses/" + QString::number( tweetData->inReplyToStatusId );
    }
    else if ( this->currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA )
      //for identica it works as is
      inReplyToUrl = "http://identi.ca/notice/" + QString::number( tweetData->inReplyToStatusId );

    m_ui->timeStamp->setText( m_ui->timeStamp->text().append( " " ).append( tr( "in reply to %1")
                                                                            .arg( QString( "<a href=%1>%2</a>" )
                                                                                  .arg( inReplyToUrl, tweetData->inReplyToScreenName ) ) ) );
  }


  setState( status.state );
  setupMenu();
  adjustSize();
}

void Tweet::setImage( const QPixmap &pixmap )
{
  m_ui->userImage->setPixmap( pixmap );
}

void Tweet::setState( TweetModel::TweetState state )
{
  tweetState = state;
  applyTheme();
}

TweetModel::TweetState Tweet::getState() const
{
  return tweetState;
}

ThemeData Tweet::getTheme()
{
  return currentTheme;
}

void Tweet::setTheme( const ThemeData &theme )
{
  currentTheme = theme;
}

void Tweet::applyTheme()
{
  switch ( tweetState ) {
  case TweetModel::STATE_UNREAD:
    setStyleSheet( currentTheme.unread.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.unread.linkColor );
    break;
  case TweetModel::STATE_ACTIVE:
    setStyleSheet( currentTheme.active.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.active.linkColor );
    break;
  case TweetModel::STATE_READ:
    setStyleSheet( currentTheme.read.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.read.linkColor );
    break;
  case TweetModel::STATE_DISABLED:
    setStyleSheet( currentTheme.disabled.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.disabled.linkColor );
  }
}

void Tweet::retranslateUi()
{
  replyAction->setText( tr( "Reply to %1" ).arg( tweetData->login ) );
  retweetAction->setText( tr( "Retweet" ) );
  copylinkAction->setText( tr( "Copy link to this tweet" ) );
  deleteAction->setText( tr( "Delete tweet" ) );
  markallasreadAction->setText( tr( "Mark all as read" ) );
  gotohomepageAction->setText( tr( "Go to User's homepage" ) );
  if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA ) {
    gototwitterpageAction->setText( tr( "Go to User's Identi.ca page" ) );
  } else {
    gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
  }
}

int Tweet::getId() const
{
  return tweetData->id;
}

// static
void Tweet::setScrollBarWidth( int width )
{
  scrollBarWidth = width;
}
void Tweet::setCurrentWidth( int width )
{
  currentWidth = width;
}
void Tweet::setCurrentLogin( const QString &login )
{
  currentLogin = login;
}
void Tweet::setCurrentNetwork( TwitterAPI::SocialNetwork network )
{
  currentNetwork = network;
}
// static_end

// TODO: magic numbers!!!!!!!!
void Tweet::adjustSize()
{
  m_ui->menuButton->move( this->size().width() - m_ui->menuButton->size().width() - 6, 6);
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->timeStamp->move( m_ui->timeStamp->x(), m_ui->userStatus->geometry().y() + m_ui->userStatus->height() );
  m_ui->frame->resize( m_ui->frame->width(), qMax(91, m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + m_ui->timeStamp->height() + 11) );
  resize( m_ui->frame->size() );
}

void Tweet::slotReply()
{
  emit reply( tweetData->login, tweetData->id );
}

void Tweet::slotRetweet()
{
  emit retweet( QString("RT @" + tweetData->login + ": " + tweetData->originalText ) );
}

void Tweet::slotCopyLink()
{
  if ( currentNetwork == TwitterAPI::SOCIALNETWORK_TWITTER )
    QApplication::clipboard()->setText( "http://twitter.com/" + tweetData->login + "/statuses/" + QString::number( tweetData->id ) );
  else if ( currentNetwork == TwitterAPI::SOCIALNETWORK_IDENTICA )
    QApplication::clipboard()->setText( "http://identi.ca/notice/" + QString::number( tweetData->id ) );
}

void Tweet::changeEvent( QEvent *e )
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    //timestamp label gets cleared after retranslation, so we need to set it again:
    if( tweetData->localTime.date() >= QDateTime::currentDateTime().date()) //today
      m_ui->timeStamp->setText( tweetData->localTime.time().toString(Qt::SystemLocaleShortDate) );
    else  //yesterday or earlier
      m_ui->timeStamp->setText( tweetData->localTime.toString(Qt::SystemLocaleShortDate) );

    break;
  default:;
  }
}

void Tweet::enterEvent( QEvent *e )
{
  if ( tweetData ) {
    if ( tweetData->isOwn )
      m_ui->menuButton->setIcon( QIcon( ":/icons/cancel_48.png" ) );
    // TODO: enable replying to public timeline statuses
    else if ( currentLogin != TwitterAPI::PUBLIC_TIMELINE )
      m_ui->replyButton->setIcon( QIcon( ":/icons/reply.png" ) );
  }
  QWidget::enterEvent( e );
}

void Tweet::leaveEvent( QEvent *e )
{
  m_ui->menuButton->setIcon( QIcon() );
  m_ui->replyButton->setIcon( QIcon() );
  QWidget::leaveEvent( e );
}

void Tweet::mousePressEvent( QMouseEvent *e )
{
  emit focusRequest();
  if ( e->button() == Qt::RightButton && tweetData ) {
    menu->exec( QCursor::pos() );
  }
}

void Tweet::focusRequest()
{
  emit selectMe( this );
}

void Tweet::slotDelete()
{
  tweetListModel->sendDeleteRequest( tweetData->id );
}


/*! \class Tweet
    \brief A widget representation of an Entry class.

    This widget class contains the status data displayed in a custom widget that is put
    on the status list view.
*/

/*! \enum Tweet::State
    \brief A Tweet's state.

    Used to specify the Tweet's current state, based on the User's selection.
*/

/*! \var Tweet::State Tweet::Unread
    The Tweet is unread.
*/

/*! \var Tweet::State Tweet::Read
    The Tweet is read.
*/

/*! \var Tweet::State Tweet::Active
    The Tweet is active, i.e. currently highlighted.
*/

/*! \fn explicit Tweet::Tweet( const Entry &entry, const QPixmap &image, QWidget *parent = 0 )
    Creates a new Tweet with a given \a parent, fills its data with the given
    \a entry and sets its user image to \a image.
*/

/*! \fn virtual Tweet::~Tweet()
    A virtual destructor.
*/

/*! \fn void Tweet::resize( const QSize& size )
    Resizes a Tweet to the given \a size.
    \param size New size of the Tweet.
    \sa adjustSize()
*/

/*! \fn void Tweet::resize( int w, int h )
    Resizes a Tweet to the width given by \a w and height given by \a h.
    Invokes adjustSize() to additionally correct the height of the Tweet.
    \param w New width of the Tweet.
    \param h New height of the Tweet.
    \sa adjustSize()
*/

/*! \fn void Tweet::setIcon( const QPixmap &image )
    Sets the Tweet's User profile image to \a image.
    \param image An image to be set.
*/

/*! \fn void Tweet::applyTheme()
    Applies a theme to the Tweet, according to its current state.
    \sa setState(), getState()
*/

/*! \fn void Tweet::retranslateUi()
    Retranslates all the translatable GUI elements of the Tweet.
    Used when changing UI language on the fly.
*/

/*! \fn bool Tweet::isRead() const
    Used to figure out if the Tweet is already read.
    \returns True when Tweet's state is \ref Read or \ref Active, false when Tweet's
             state is \ref Unread.
    \sa getState()
*/

/*! \fn Tweet::State Tweet::getState() const
    Used to figure out the actual state of the Tweet.
    \returns The current state of the Tweet.
    \sa setState(), isRead()
*/

/*! \fn void Tweet::setState( Tweet::State state );
    Sets Tweet's current state to \a state.
    \param state A \ref State to set for the Tweet.
*/

/*! \fn static ThemeData Tweet::getTheme();
    Provides information about the theme that is currently set to all the Tweets.
    \returns Current Tweets' theme.
    \sa setTheme()
*/

/*! \fn static void Tweet::setTheme( const ThemeData &theme )
    Sets the current theme for all the Tweets.
    \param theme A theme to be set.
    \sa getTheme()
*/

/*! \fn static void Tweet::setTweetListModel( TweetModel *tweetModel )
    Sets a Tweet list model. The model is used by a Tweet to receive signals
    from a Tweet menu.
    \param tweetModel The model to be set.
*/

/*! \fn void Tweet:adjustSize()
    Adjusts height of the Tweet widget to fit the whole status.
    \sa resize()
*/

/*! \fn void Tweet::menuRequested()
    Opens a Tweet menu.
*/

/*! \fn void Tweet::slotReply()
    Prepares and emits a \ref reply() signal.
    \sa reply()
*/

/*! \fn void Tweet::slotRetweet()
    Prepares a retweet (citation of other user's status) and emits a
    \ref retweet() signal to post it.
    \sa retweet()
*/

/*! \fn void Tweet::slotCopyLink()
    Copies a link to the Tweet to system clipboard.
*/

/*! \fn void Tweet::reply( const QString &name, int inReplyTo )
    Emitted to notify the MainWindow class instance about the User's request
    to send a reply.
    \param name Login of the User to whom a reply is addressed.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa slotReply()
*/

/*! \fn void Tweet::retweet( QString message )
    Emitted to notify the Core class instance about the User's request
    to retweet a status.
    \param message A retweet status message prepared by \ref slotRetweet().
    \sa slotRetweet()
*/

/*! \fn void Tweet::markAllAsRead()
    Emitted to notify the TweetModel class instance about User's request
    to mark all the Tweets in a list as read.
*/

/*! \fn void Tweet::selectMe( Tweet *tweet )
    Emitted to notify the TweetModel class instance about User's request
    to select (highlight) the current Tweet.
    \param tweet A Tweet to be highlighted. Usually \a this.
*/

/*! \fn void Tweet::changeEvent( QEvent *e )
    Reimplemented to force \ref retranslateUi() upon a language change.
    \param e A QEvent event's representation.
    \sa retranslateUi()
*/

/*! \fn void Tweet::enterEvent( QEvent *e )
    Reimplemented to show the menu icon when hovered by mouse.
    \param e A QEvent event's representation.
*/

/*! \fn void Tweet::leaveEvent( QEvent *e )
    Reimplemented to hide the menu icon when mouse leaves the Tweet.
    \param e A QEvent event's representation.
*/
