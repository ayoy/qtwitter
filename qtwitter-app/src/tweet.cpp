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


#include "tweet.h"
#include "ui_tweet.h"
#include <QDebug>
#include <QProcess>
#include <QClipboard>

ThemeData Tweet::currentTheme = ThemeData();
TweetModel* Tweet::tweetListModel = 0;

Tweet::Tweet( const Entry &entry, const QImage &image, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  tweetState( Tweet::Unread ),
  tweetData( entry ),
  m_ui(new Ui::Tweet)
{
  menu = new QMenu( this );
  menuFont = new QFont( menu->font() );
  menuFont->setPixelSize( 10 );
  menu->setFont( *menuFont );

  signalMapper = new QSignalMapper( this );

  replyAction = new QAction( tr("Reply to %1" ).arg( tweetData.login() ), this);
  menu->addAction( replyAction );
  replyAction->setFont( *menuFont );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(sendReply()) );
  connect( this, SIGNAL(reply(QString,int)), tweetListModel, SIGNAL(reply(QString,int)) );
  if ( tweetData.getType() != Entry::Status ) {
    replyAction->setEnabled( false );
  }

  retweetAction = new QAction( tr( "Retweet" ), this );
  menu->addAction( retweetAction );
  retweetAction->setFont( *menuFont );
  connect( retweetAction, SIGNAL(triggered()), this, SLOT(sendRetweet()) );
  connect( this, SIGNAL(retweet(QString)), tweetListModel, SIGNAL(retweet(QString)) );

  menu->addSeparator();

  copylinkAction = new QAction( tr( "Copy link to this Tweet" ), this );
  menu->addAction( copylinkAction );
  copylinkAction->setFont( *menuFont );
  connect( copylinkAction, SIGNAL(triggered()), this, SLOT(copyLink()) );
  if ( tweetData.getType() != Entry::Status ) {
    copylinkAction->setEnabled( false );
  }

  deleteAction = new QAction( tr( "Delete tweet" ), this );
  menu->addAction( deleteAction );
  deleteAction->setFont( *menuFont );
  signalMapper->setMapping( deleteAction, tweetData.id() );
  connect( deleteAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(int)), tweetListModel, SIGNAL(destroy(int)) );
  if ( !tweetData.isOwn() ) {
    deleteAction->setEnabled( false );
  }

  markallasreadAction = new QAction( tr( "Mark all as read" ), this );
  menu->addAction( markallasreadAction );
  markallasreadAction->setFont( *menuFont );
  connect( markallasreadAction, SIGNAL(triggered()), tweetListModel, SLOT(markAllAsRead()) );

  menu->addSeparator();

  gototwitterpageAction = new QAction( tr( "Go to User's Twitter page" ), this );
  menu->addAction( gototwitterpageAction );
  gototwitterpageAction->setFont( *menuFont );
  signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + tweetData.login() );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), tweetListModel, SLOT(emitOpenBrowser(QString)) );

  gotohomepageAction = new QAction( tr( "Go to User's homepage" ), this);
  menu->addAction( gotohomepageAction );
  gotohomepageAction->setFont( *menuFont );
  signalMapper->setMapping( gotohomepageAction, tweetData.homepage() );
  connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  if ( !tweetData.homepage().compare("") ) {
    gotohomepageAction->setEnabled( false );
  }

  menu->addSeparator();

  aboutAction = new QAction( tr( "About qTwitter..." ), this );
  menu->addAction( aboutAction );
  aboutAction->setFont( *menuFont );
  connect( aboutAction, SIGNAL(triggered()), tweetListModel, SIGNAL(about()) );

  m_ui->setupUi( this );

  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(Tweet*)), tweetListModel, SLOT(selectTweet(Tweet*)) );

  applyTheme();
  m_ui->userName->setText( tweetData.name() );
  m_ui->userStatus->setHtml( tweetData.text() );
  m_ui->userImage->setPixmap( QPixmap::fromImage( image ) );
  adjustSize();
  this->setFocusProxy( m_ui->userStatus );
  connect( m_ui->menuButton, SIGNAL(pressed()), this, SLOT(menuRequested()) );
}

Tweet::~Tweet()
{
  delete m_ui;
  m_ui = 0;
  delete menuFont;
  menuFont = 0;
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

void Tweet::setIcon( const QImage &image )
{
  m_ui->userImage->setPixmap( QPixmap::fromImage( image ) );
}

void Tweet::applyTheme()
{
  switch ( tweetState ) {
  case Tweet::Unread:
    setStyleSheet( currentTheme.unread.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.unread.linkColor );
    break;
  case Tweet::Active:
    setStyleSheet( currentTheme.active.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.active.linkColor );
    break;
  case Tweet::Read:
    setStyleSheet( currentTheme.read.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.read.linkColor );
  }
  m_ui->userStatus->setHtml( tweetData.text() );
  this->update();
}

void Tweet::retranslateUi()
{
  replyAction->setText( tr( "Reply to %1" ).arg( tweetData.login() ) );
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
  if ( tweetState == Tweet::Unread )
    return false;
  return true;
}

Tweet::State Tweet::getState() const
{
  return tweetState;
}

void Tweet::setState( Tweet::State state )
{
  tweetState = state;
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

void Tweet::setTweetListModel( TweetModel *tweetModel )
{
  tweetListModel = tweetModel;
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
  emit reply( tweetData.login(), tweetData.id() );
}

void Tweet::sendRetweet()
{
  emit retweet( QString("RT @" + tweetData.login() + ": " + tweetData.originalText() ) );
}

void Tweet::copyLink()
{
  QApplication::clipboard()->setText( "http://twitter.com/" + tweetData.login() + "/statuses/" + QString::number( tweetData.id() ) );
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
