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


#include <QDebug>
#include <QClipboard>
#include <QProcess>
#include "tweet.h"
#include "ui_tweet.h"
#include "settings.h"

ThemeData Tweet::currentTheme = ThemeData();

Tweet::Tweet( Entry *entry, TweetModel::TweetState *state, const QImage &image, TweetModel *parentModel, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  tweetState( state ),
  tweetData( entry ),
  tweetListModel( parentModel ),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi( this );

  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(Tweet*)), tweetListModel, SLOT(selectTweet(Tweet*)) );

  applyTheme();
  m_ui->userName->setText( tweetData->name );
  m_ui->userStatus->setHtml( tweetData->text );
  m_ui->userImage->setPixmap( QPixmap::fromImage( image ) );
  adjustSize();
  setFocusProxy( m_ui->userStatus );
  connect( m_ui->menuButton, SIGNAL(pressed()), this, SLOT(menuRequested()) );
  createMenu();
}

Tweet::~Tweet()
{
  delete m_ui;
}

void Tweet::createMenu()
{
  menu = new QMenu( this );
  QFont menuFont;
  menuFont.setPixelSize( 10 );
  menu->setFont( menuFont );

  signalMapper = new QSignalMapper( this );

  replyAction = new QAction( tr("Reply to %1" ).arg( tweetData->login ), this);
  menu->addAction( replyAction );
  replyAction->setFont( menuFont );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(sendReply()) );
  connect( this, SIGNAL(reply(QString,int)), tweetListModel, SIGNAL(reply(QString,int)) );
  if ( tweetData->type != Entry::Status ) {
    replyAction->setEnabled( false );
  }

  retweetAction = new QAction( tr( "Retweet" ), this );
  menu->addAction( retweetAction );
  retweetAction->setFont( menuFont );
  connect( retweetAction, SIGNAL(triggered()), this, SLOT(sendRetweet()) );
  connect( this, SIGNAL(retweet(QString)), tweetListModel, SIGNAL(retweet(QString)) );

  menu->addSeparator();

  copylinkAction = new QAction( tr( "Copy link to this Tweet" ), this );
  menu->addAction( copylinkAction );
  copylinkAction->setFont( menuFont );
  connect( copylinkAction, SIGNAL(triggered()), this, SLOT(copyLink()) );
  if ( tweetData->type != Entry::Status ) {
    copylinkAction->setEnabled( false );
  }

  deleteAction = new QAction( tr( "Delete tweet" ), this );
  menu->addAction( deleteAction );
  deleteAction->setFont( menuFont );
  signalMapper->setMapping( deleteAction, tweetData->id );
  connect( deleteAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(int)), tweetListModel, SIGNAL(destroy(int)) );
  if ( !tweetData->isOwn ) {
    deleteAction->setEnabled( false );
  }

  markallasreadAction = new QAction( tr( "Mark all as read" ), this );
  menu->addAction( markallasreadAction );
  markallasreadAction->setFont( menuFont );
  connect( markallasreadAction, SIGNAL(triggered()), tweetListModel, SLOT(markAllAsRead()) );

  menu->addSeparator();

  gototwitterpageAction = new QAction( tr( "Go to User's Twitter page" ), this );
  menu->addAction( gototwitterpageAction );
  gototwitterpageAction->setFont( menuFont );
  signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + tweetData->login );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), tweetListModel, SLOT(emitOpenBrowser(QString)) );

  gotohomepageAction = new QAction( tr( "Go to User's homepage" ), this);
  menu->addAction( gotohomepageAction );
  gotohomepageAction->setFont( menuFont );
  signalMapper->setMapping( gotohomepageAction, tweetData->homepage );
  connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  if ( !tweetData->homepage.compare("") ) {
    gotohomepageAction->setEnabled( false );
  }

  menu->addSeparator();

  aboutAction = new QAction( tr( "About qTwitter..." ), this );
  menu->addAction( aboutAction );
  aboutAction->setFont( menuFont );
  connect( aboutAction, SIGNAL(triggered()), tweetListModel, SIGNAL(about()) );
}

void Tweet::resize( const QSize &s )
{
  m_ui->frame->resize( s );
  QWidget::resize( s );
}

void Tweet::resize( int w, int h )
{
  QWidget::resize( w, h );
  m_ui->frame->resize( w, h );
  m_ui->userStatus->resize( size().width() - m_ui->userStatus->geometry().x() - 18, m_ui->userStatus->size().height() );
  adjustSize();
}

void Tweet::setTweetData( Entry *entry, TweetModel::TweetState *state )
{
  tweetData = entry;
  tweetState = state;
}

void Tweet::setIcon( const QImage &image )
{
  m_ui->userImage->setPixmap( QPixmap::fromImage( image ) );
}

void Tweet::applyTheme()
{
  switch ( *tweetState ) {
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
  }
  m_ui->userStatus->setHtml( tweetData->text );
  this->update();
}

void Tweet::retranslateUi()
{
  replyAction->setText( tr( "Reply to %1" ).arg( tweetData->login ) );
  retweetAction->setText( tr( "Retweet" ) );
  copylinkAction->setText( tr( "Copy link to this Tweet" ) );
  deleteAction->setText( tr( "Delete tweet" ) );
  markallasreadAction->setText( tr( "Mark all as read" ) );
  gotohomepageAction->setText( tr( "Go to User's homepage" ) );
  gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
  aboutAction->setText( tr( "About qTwitter..." ) );
}

bool Tweet::isRead() const
{
  if ( *tweetState == TweetModel::STATE_UNREAD )
    return false;
  return true;
}

TweetModel::TweetState Tweet::getState() const
{
  return *tweetState;
}

void Tweet::setState( TweetModel::TweetState state )
{
  *tweetState = state;
  applyTheme();
}

ThemeData Tweet::getTheme()
{
  return currentTheme;
}

void Tweet::setTheme( const ThemeData &theme )
{
  currentTheme = theme;
}

void Tweet::adjustSize()
{
  m_ui->menuButton->move( this->size().width() - m_ui->menuButton->size().width() - 6, 6);
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->frame->resize( m_ui->frame->width(), ( 68 > m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11) ? 68 : m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11 );
  resize( m_ui->frame->size() );
}

void Tweet::menuRequested()
{
  emit focusRequest();
  menu->exec( QCursor::pos() );
}

void Tweet::sendReply()
{
  emit reply( tweetData->login, tweetData->id );
}

void Tweet::sendRetweet()
{
  emit retweet( QString("RT @" + tweetData->login + ": " + tweetData->originalText ) );
}

void Tweet::copyLink()
{
  QApplication::clipboard()->setText( "http://twitter.com/" + tweetData->login + "/statuses/" + QString::number( tweetData->id ) );
}

void Tweet::changeEvent( QEvent *e )
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:;
  }
}

void Tweet::enterEvent( QEvent *e )
{
  m_ui->menuButton->setIcon( QIcon( ":/icons/star_48.png" ) );
  QWidget::enterEvent( e );
}

void Tweet::leaveEvent( QEvent *e )
{
  m_ui->menuButton->setIcon( QIcon() );
  QWidget::leaveEvent( e );
}

void Tweet::focusRequest()
{
  emit selectMe( this );
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

/*! \fn explicit Tweet::Tweet( const Entry &entry, const QImage &image, QWidget *parent = 0 )
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

/*! \fn void Tweet::setIcon( const QImage &image )
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

/*! \fn void Tweet::sendReply()
    Prepares and emits a \ref reply() signal.
    \sa reply()
*/

/*! \fn void Tweet::sendRetweet()
    Prepares a retweet (citation of other user's status) and emits a
    \ref retweet() signal to post it.
    \sa retweet()
*/

/*! \fn void Tweet::copyLink()
    Copies a link to the Tweet to system clipboard.
*/

/*! \fn void Tweet::reply( const QString &name, int inReplyTo )
    Emitted to notify the MainWindow class instance about the User's request
    to send a reply.
    \param name Login of the User to whom a reply is addressed.
    \param inReplyTo Id of the existing status to which the reply is posted.
    \sa sendReply()
*/

/*! \fn void Tweet::retweet( QString message )
    Emitted to notify the Core class instance about the User's request
    to retweet a status.
    \param message A retweet status message prepared by \ref sendRetweet().
    \sa sendRetweet()
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
