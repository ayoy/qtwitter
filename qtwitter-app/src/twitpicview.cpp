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
  connect( m_ui->imagePathEdit, SIGNAL(textChanged(QString)), this, SLOT(setImagePreview(QString)) );
  connect( m_ui->browseButton, SIGNAL(pressed()), this, SLOT(setImagePath()) );

#if QT_VERSION < 0x040500
  m_ui->statusEdit->setMaximumWidth( 150 );
#endif

}

TwitPicView::~TwitPicView()
{
  delete m_ui;
  if ( pixmap ) {
    delete pixmap;
    pixmap = 0;
  }
}

void TwitPicView::showUploadProgress( int done, int total )
{
  m_ui->uploadProgressBar->setMaximum( total );
  m_ui->uploadProgressBar->setValue( done );
}

void TwitPicView::resetForm()
{
  m_ui->uploadButton->setText( tr( "Upload" ) );
  m_ui->uploadProgressBar->setValue( 0 );
  m_ui->uploadProgressBar->setVisible( false );
  m_ui->imagePathEdit->clear();
  m_ui->statusEdit->clear();
}

void TwitPicView::reject()
{
  emit abortUpload();
  resetForm();
  QDialog::reject();
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
    if ( pixmap->width() > m_ui->imagePreview->width() || pixmap->height() > m_ui->imagePreview->height() ) {
      m_ui->imagePreview->setPixmap( pixmap->scaled( m_ui->imagePreview->size(), Qt::KeepAspectRatio ) );
    } else {
      m_ui->imagePreview->setPixmap( *pixmap );
    }
  }
  QDialog::resizeEvent( e );
}

void TwitPicView::sendUploadRequest()
{
  if ( m_ui->uploadButton->text() == tr( "Upload" ) ) {
    if ( m_ui->postStatusCheckBox->isChecked() ) {
      emit uploadPhoto( m_ui->imagePathEdit->text(), m_ui->statusEdit->toPlainText() );
    } else {
      emit uploadPhoto( m_ui->imagePathEdit->text(), QString() );
    }
    m_ui->uploadButton->setText( tr( "Abort" ) );
    m_ui->uploadProgressBar->setVisible( true );
  } else if ( m_ui->uploadButton->text() == tr( "Abort" ) ) {
    emit abortUpload();
    m_ui->uploadButton->setText( tr( "Upload" ) );
    m_ui->uploadProgressBar->setValue( 0 );
    m_ui->uploadProgressBar->setVisible( false );
  }
}

void TwitPicView::setImagePath()
{
  m_ui->imagePathEdit->setText( QFileDialog::getOpenFileName( this, tr( "Select photo to upload" ), getHomeDir(), tr( "Image files" ).append( " (*.jpg *.jpeg *.png *.bmp *.gif)" ) ) );
}

void TwitPicView::setImagePreview( const QString &path )
{
  if ( path.isEmpty() ) {
    m_ui->imagePreview->setText( tr( "Select a photo to upload" ) );
    m_ui->uploadButton->setEnabled( false );
    return;
  }
  if ( !QFileInfo( path ).isFile() ) {
    m_ui->imagePreview->setText( tr( "Select a photo to upload" ) );
    m_ui->uploadButton->setEnabled( false );
    return;
  }
  if ( pixmap ) {
    delete pixmap;
    pixmap = NULL;
  }
  pixmap = new QPixmap( QPixmap::fromImage( QImage( path ) ) );
  if ( pixmap->isNull() ) {
    delete pixmap;
    pixmap = NULL;
    m_ui->imagePreview->setText( tr( "Select a photo to upload" ) );
    m_ui->uploadButton->setEnabled( false );
    return;
  }
  if ( pixmap->width() > m_ui->imagePreview->width() || pixmap->height() > m_ui->imagePreview->height() ) {
    m_ui->imagePreview->setPixmap( pixmap->scaled( m_ui->imagePreview->size(), Qt::KeepAspectRatio ) );
  } else {
    m_ui->imagePreview->setPixmap( *pixmap );
  }
  m_ui->uploadButton->setEnabled( true );
}

QString TwitPicView::getHomeDir()
{
  QRegExp rx( ";HOME=(.+);", Qt::CaseSensitive );
  rx.setMinimal( true );
  rx.indexIn( QProcess::systemEnvironment().join( ";" ) );
  return rx.cap(1);
}
