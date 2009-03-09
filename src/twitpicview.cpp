/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include "twitpicview.h"
#include "ui_twitpicview.h"

TwitPicView::TwitPicView(QWidget *parent) :
    QDialog(parent),
    pixmap( 0 ),
    m_ui(new Ui::TwitPicView)
{
  m_ui->setupUi(this);
  m_ui->uploadProgressBar->setVisible(false);
  connect( m_ui->uploadButton, SIGNAL(clicked()), this, SLOT(sendUploadRequest()) );
  connect( m_ui->browseButton, SIGNAL(pressed()), this, SLOT(setImagePath()) );
}

TwitPicView::~TwitPicView()
{
  delete m_ui;
}

void TwitPicView::changeEvent(QEvent *e)
{
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void TwitPicView::resizeEvent( QResizeEvent *e )
{
  if ( m_ui && m_ui->imagePreview->pixmap() ) {
    m_ui->imagePreview->setPixmap( pixmap->scaled( m_ui->imagePreview->size(), Qt::KeepAspectRatio ) );
  }
  QDialog::resizeEvent( e );
}

void TwitPicView::sendUploadRequest()
{
  if ( m_ui->postStatusCheckBox->isChecked() ) {
    emit uploadPhoto( m_ui->imagePathEdit->text(), m_ui->statusEdit->toPlainText() );
  } else {
    emit uploadPhoto( m_ui->imagePathEdit->text(), QString() );
  }
  m_ui->uploadButton->setText( tr( "Abort" ) );
}

void TwitPicView::setImagePath()
{
  m_ui->imagePathEdit->setText( QFileDialog::getOpenFileName( this, tr( "Select photo to upload" ), getHomeDir(), tr( "Image files (*.jpg *.jpeg *.png *.bmp *.gif)" ) ) );
  if ( m_ui->imagePathEdit->text().isEmpty() ) {
    m_ui->imagePreview->setText( tr( "Select a photo to upload" ) );
    m_ui->uploadButton->setEnabled( false );
    return;
  }
  if ( pixmap ) {
    delete pixmap;
  }
  pixmap = new QPixmap( QPixmap::fromImage( QImage( m_ui->imagePathEdit->text() ) ) );
  m_ui->imagePreview->setPixmap( pixmap->scaled( m_ui->imagePreview->size(), Qt::KeepAspectRatio ) );
  m_ui->uploadButton->setEnabled( true );
}

QString TwitPicView::getHomeDir()
{
  QRegExp rx( ";HOME=(.+);", Qt::CaseSensitive );
  rx.setMinimal( true );
  rx.indexIn( QProcess::systemEnvironment().join( ";" ) );
  return rx.cap(1);
}
