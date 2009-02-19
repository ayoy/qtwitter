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

const QString Tweet::STYLESHEET_CARAMEL = QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(101, 93, 86, 255), stop:0.0150754 rgba(23, 14, 40, 255), stop:1 rgba(112, 99, 37, 255)); border-width: 3px; border-style: outset; border-color: rgb(219, 204, 56); border-radius: 10px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal}" );
const QString Tweet::STYLESHEET_SKY = QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 187, 218, 255), stop:1 rgba(222, 231, 255, 255)); border-width: 3px; border-style: outset; border-color: rgb(203, 239, 255); border-radius: 10px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(0, 60, 196); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(0, 60, 196); border-width: 0px; border-style: normal }" );


Tweet::Tweet( const Entry &entry, const QImage &icon, const QString &style, MainWindow *parent ) :
  QWidget(parent),
  gotohomepageAction(0),
  model( entry ),
  parentMainWindow(parent),
  m_ui(new Ui::Tweet)
{
  menu = new QMenu( this );
  menuFont = new QFont( menu->font() );
  menuFont->setPixelSize( 10 );
  menu->setFont( *menuFont );

  replyAction = new QAction( tr("Reply to") + " " + model.login(), this);
  menu->addAction( replyAction );
  connect( replyAction, SIGNAL(triggered()), this, SLOT(sendReply()) );
  connect( this, SIGNAL(reply(QString)), parent, SIGNAL(addReplyString(QString)) );

  signalMapper = new QSignalMapper( this );
  gototwitterpageAction = new QAction( tr( "Go to User's Twitter page" ), this );
  menu->addAction( gototwitterpageAction );
  signalMapper->setMapping( gototwitterpageAction, "http://twitter.com/" + model.login() );
  connect( gototwitterpageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
  connect( signalMapper, SIGNAL(mapped(QString)), parentMainWindow, SIGNAL(openBrowser(QString)) );

  if ( model.homepage().compare("") ) {
    gotohomepageAction = new QAction( tr("Go to User's homepage"), this);
    menu->addAction( gotohomepageAction );
    gotohomepageAction->setFont( *menuFont );
    signalMapper->setMapping( gotohomepageAction, model.homepage() );
    connect( gotohomepageAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
    connect( signalMapper, SIGNAL(mapped(QString)), parentMainWindow, SIGNAL(openBrowser(QString)) );
  }

  if ( model.isOwn() ) {
    deleteAction = new QAction( tr( "Delete tweet" ), this );
    menu->addAction( deleteAction );
    deleteAction->setFont( *menuFont );
    signalMapper->setMapping( deleteAction, model.id() );
    connect( deleteAction, SIGNAL(triggered()), signalMapper, SLOT(map()) );
    connect( signalMapper, SIGNAL(mapped(int)), parentMainWindow, SIGNAL(destroy(int)) );
  }

  replyAction->setFont( *menuFont );
  gototwitterpageAction->setFont( *menuFont );

  m_ui->setupUi( this );
  applyStyle( style );
  m_ui->userName->setText( model.name() );
  m_ui->userStatus->setHtml( model.text() );
  m_ui->userIcon->setPixmap( QPixmap::fromImage( icon ) );
  adjustSize();
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

void Tweet::applyStyle( const QString &style )
{
  if ( style == "Caramel" ) {
    setStyleSheet( STYLESHEET_CARAMEL );
    m_ui->userStatus->document()->setDefaultStyleSheet( "a { color: rgb(255, 248, 140); }" );
    m_ui->userStatus->setHtml( model.text() );
  } else {
    setStyleSheet( STYLESHEET_SKY );
    m_ui->userStatus->document()->setDefaultStyleSheet( "a { color: rgb(0, 0, 255); }" );
    m_ui->userStatus->setHtml( model.text() );
  }
  this->update();
}

void Tweet::retranslateUi()
{
  replyAction->setText( tr("Reply to") + " " + model.login() );
  if ( gotohomepageAction ) {
    gotohomepageAction->setText( tr("Go to homepage") + QString(" (%1)").arg( model.homepage() ) );
  }
}
