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
#include "settings.h"
#include <QDebug>
#include <QProcess>
#include <QClipboard>

ThemeData Tweet::currentTheme = ThemeData();
TweetModel* Tweet::tweetListModel = 0;

Tweet::Tweet( const Entry &entry, const QImage &icon, QWidget *parent ) :
  QWidget(parent),
  replyAction(0),
  gotohomepageAction(0),
  gototwitterpageAction(0),
  deleteAction(0),
  model( entry ),
  read( false ),
  m_ui(new Ui::Tweet)
{
  menu = new QMenu( this );
  menuFont = new QFont( menu->font() );
  menuFont->setPixelSize( 10 );
  menu->setFont( *menuFont );

  signalMapper = new QSignalMapper( this );

  replyAction = new QAction( tr("Reply to") + " " + model.login(), this);
  menu->addAction( replyAction );
  replyAction->setFont( *menuFont );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(sendReply()) );
  connect( this, SIGNAL(reply(QString)), tweetListModel, SIGNAL(addReplyString(QString)) );
  if ( model.getType() != Entry::Status ) {
    replyAction->setEnabled( false );
  }

  retweetAction = new QAction( tr( "Retweet" ), this );
  menu->addAction( retweetAction );
  retweetAction->setFont( *menuFont );
  connect( retweetAction, SIGNAL(triggered()), this, SLOT(retweet()) );
  connect( this, SIGNAL(postRetweet(QByteArray)), tweetListModel, SIGNAL(postRetweet(QByteArray)) );

  menu->addSeparator();

  copylinkAction = new QAction( tr( "Copy link to this Tweet" ), this );
  menu->addAction( copylinkAction );
  copylinkAction->setFont( *menuFont );
  connect( copylinkAction, SIGNAL(triggered()), this, SLOT(copyLink()) );
  if ( model.getType() != Entry::Status ) {
    copylinkAction->setEnabled( false );
  }

  deleteAction = new QAction( tr( "Delete tweet" ), this );
  menu->addAction( deleteAction );
  deleteAction->setFont( *menuFont );
  signalMapper->setMapping( deleteAction, model.id() );
  connect( deleteAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(int)), tweetListModel, SIGNAL(destroy(int)) );
  if ( !model.isOwn() ) {
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
  signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + model.login() );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), tweetListModel, SIGNAL(openBrowser(QString)) );

  gotohomepageAction = new QAction( tr( "Go to User's homepage" ), this);
  menu->addAction( gotohomepageAction );
  gotohomepageAction->setFont( *menuFont );
  signalMapper->setMapping( gotohomepageAction, model.homepage() );
  connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  if ( !model.homepage().compare("") ) {
    gotohomepageAction->setEnabled( false );
  }

  menu->addSeparator();

  aboutAction = new QAction( tr( "About qTwitter..." ), this );
  menu->addAction( aboutAction );
  aboutAction->setFont( *menuFont );
  connect( aboutAction, SIGNAL(triggered()), tweetListModel, SIGNAL(about()) );

  m_ui->setupUi( this );

  connect( m_ui->userStatus, SIGNAL(mousePressed()), this, SLOT(focusRequest()) );
  connect( this, SIGNAL(selectMe(Tweet*)), tweetListModel, SLOT(select(Tweet*)) );

  applyTheme();
  m_ui->userName->setText( model.name() );
  m_ui->userStatus->setHtml( model.text() );
  m_ui->userIcon->setPixmap( QPixmap::fromImage( icon ) );
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

QString Tweet::getUrlForIcon() const
{
  return model.image();
}

void Tweet::menuRequested()
{
  menu->exec( QCursor::pos() );
}

void Tweet::copyLink()
{
  QApplication::clipboard()->setText( "http://twitter.com/" + model.login() + "/statuses/" + QString::number( model.id() ) );
}

void Tweet::setIcon( const QImage &image )
{
  m_ui->userIcon->setPixmap( QPixmap::fromImage( image ) );
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

void Tweet::adjustSize()
{
  m_ui->menuButton->move( this->size().width() - m_ui->menuButton->size().width() - 6, 6);
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->frame->resize( m_ui->frame->width(), ( 68 > m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11) ? 68 : m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11 );
  resize( m_ui->frame->size() );
}

void Tweet::changeEvent( QEvent *e )
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
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

void Tweet::sendReply()
{
  emit reply( model.login() );
}

void Tweet::retweet()
{
  emit postRetweet( QString("RT @" + model.login() + ": " + model.originalText() ).toUtf8() );
}

void Tweet::setTheme( const ThemeData &theme )
{
  currentTheme = theme;
}

ThemeData Tweet::getTheme()
{
  return currentTheme;
}

void Tweet::setTweetListModel( TweetModel *tweetModel )
{
  tweetListModel = tweetModel;
}

void Tweet::applyTheme( Settings::ThemeVariant variant )
{
  switch ( variant ) {
  case Settings::Unread:
    setStyleSheet( currentTheme.unread.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.unread.linkColor );
    break;
  case Settings::Active:
    setStyleSheet( currentTheme.active.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.active.linkColor );
    break;
  case Settings::Read:
    setStyleSheet( currentTheme.read.styleSheet );
    m_ui->userStatus->document()->setDefaultStyleSheet( currentTheme.read.linkColor );
  }
  m_ui->userStatus->setHtml( model.text() );
  this->update();
}

bool Tweet::isRead() const
{
  return read;
}

void Tweet::markAsRead()
{
  read = true;
}

void Tweet::markAsUnread()
{
  read = false;
  applyTheme( Settings::Unread );
}

void Tweet::setRead()
{
  applyTheme( Settings::Read );
}

void Tweet::setActive()
{
  applyTheme( Settings::Active );
}

void Tweet::focusRequest()
{
  emit selectMe( this );
}

void Tweet::retranslateUi()
{
  if ( replyAction ) {
    replyAction->setText( tr("Reply to") + " " + model.login() );
  }
  if ( gotohomepageAction ) {
    gotohomepageAction->setText( tr("Go to User's homepage") );
  }
  if ( gototwitterpageAction ) {
    gototwitterpageAction->setText( tr( "Go to User's Twitter page" ) );
  }
  if ( deleteAction ) {
    deleteAction->setText( tr( "Delete tweet" ) );
  }
  aboutAction->setText( tr( "About qTwitter..." ) );
}
