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

Tweet::Tweet(QWidget *parent) :
  QWidget(parent),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi(this);
}

Tweet::Tweet( const QString &name, const QString &status, const QImage &icon, QWidget *parent ) :
  QWidget(parent),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi( this );
  m_ui->userName->setText( name );
  m_ui->userStatus->document()->setDefaultStyleSheet( "a { color: rgb(255, 248, 140); }" );
  m_ui->userStatus->setHtml( status );
  m_ui->userIcon->setPixmap( QPixmap::fromImage( icon ) );
  adjustSize();
}

Tweet::~Tweet()
{
  delete m_ui;
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
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->frame->resize( m_ui->frame->width(), ( 68 > m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11) ? 68 : m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11 );
  resize( m_ui->frame->size() );
}

void Tweet::changeEvent(QEvent *e)
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}
