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


#include <QFileDialog>
#include <QProcess>
#include <QDebug>
#include <QStringList>
#include <twitterapi/twitterapi_global.h>
#include "twitpicview.h"
#include "ui_twitpicview.h"
#include "configfile.h"

extern ConfigFile settings;

TwitPicView::TwitPicView(QWidget *parent) :
        QDialog(parent),
        pixmap( 0 ),
        m_ui(new Ui::TwitPicView)
{
    m_ui->setupUi( this );
    m_ui->uploadProgressBar->setVisible( false );
    m_ui->statusEdit->setAllowEnters( false );
    connect( m_ui->uploadButton, SIGNAL(clicked()), this, SLOT(sendUploadRequest()) );
    connect( m_ui->statusEdit, SIGNAL(enterPressed()), this, SLOT(sendUploadRequest()) );
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

void TwitPicView::setupAccounts( const QList<Account> &accounts )
{
    m_ui->accountsComboBox->clear();
    foreach ( Account account, accounts ) {
        if ( account.isEnabled() && account.serviceUrl() == Account::NetworkUrlTwitter )
            m_ui->accountsComboBox->addItem( account.login() );
    }
    int index = settings.value( "Accounts/visibleAccount", 0 ).toInt();
    if ( index >= m_ui->accountsComboBox->count() )
        m_ui->accountsComboBox->setCurrentIndex( m_ui->accountsComboBox->count() - 1 );
    else
        m_ui->accountsComboBox->setCurrentIndex( index );
}

void TwitPicView::setupAccounts( const QStringList &accounts )
{
    m_ui->accountsComboBox->clear();
    m_ui->accountsComboBox->addItems( accounts );
}


void TwitPicView::showUploadProgress( qint64 done, qint64 total )
{
    m_ui->uploadProgressBar->setMaximum( (int)total );
    m_ui->uploadProgressBar->setValue( (int)done );
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
            emit uploadPhoto( m_ui->accountsComboBox->currentText(), m_ui->imagePathEdit->text(), m_ui->statusEdit->toPlainText() );
        } else {
            emit uploadPhoto( m_ui->accountsComboBox->currentText(), m_ui->imagePathEdit->text(), QString() );
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
    m_ui->imagePathEdit->setText( QFileDialog::getOpenFileName(
            this, tr( "Select photo to upload" ),
            settings.value("TwitPic/lastPath", getHomeDir()).toString(),
            tr( "Image files" ).append( " (*.jpg *.jpeg *.png *.bmp *.gif *.JPG *.JPEG *.PNG *.GIF)" ) ) );
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
    pixmap = new QPixmap( path );
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

    settings.setValue("TwitPic/lastPath", QFileInfo( path ).path());
}

QString TwitPicView::getHomeDir()
{
    QRegExp rx( ";HOME=(.+);", Qt::CaseSensitive );
    rx.setMinimal( true );
    if ( rx.indexIn( QProcess::systemEnvironment().join( ";" ) ) != -1 )
        return rx.cap(1);
    else return QString("");
}


/*! \class TwitPicView
    \brief A GUI frontend to TwitPicEngine class.

    This class provides a dialog for uploading photos to TwitPic. It is responsible
    for communicating with User: enabling selection of a photo to upload, adding
    a status to be posted and showing the progress of an operation. Behind the scenes
    it talks to TwitPicEngine class that interacts with TwitPic REST API.
*/

/*! \fn explicit TwitPicView::TwitPicView(QWidget *parent = 0)
    Creates a new dialog with a given \a parent.
*/

/*! \fn virtual TwitPicView::~TwitPicView()
    Destroys a dialog.
*/

/*! \fn void TwitPicView::showUploadProgress( int done, int total )
    Updates the progress bar to show photo uploading progress.
    \param done Amount of data uploaded so far.
    \param total Total amount of data to be uploaded.
*/

/*! \fn void TwitPicView::resetForm()
    Resets input fields and applies widgets' initial states. Used after
    completing or aborting upload.
    \sa abortUpload()
*/

/*! \fn void TwitPicView::reject()
    Reimplemented from QDialog to handle aborting the request when closing the window (rejecting the dialog).
*/

/*! \fn void TwitPicView::uploadPhoto( QString photoPath, QString status )
    Emitted when User presses the "Upload" button. Notifies TwitPicEngine class of the upload request.
    \param photoPath A path to the image on disk to be uploaded.
    \param status An optional status to be posted to Twitter together with a link to image.
*/

/*! \fn void TwitPicView::abortUpload()
    Emitted when User presses the "Abort" button. Notifies TwitPicEngine about cancelling upload.
*/

/*! \fn virtual void TwitPicView::changeEvent( QEvent *e )
    Reimplemented from QDialog, enables on-the fly widget's translation.
*/

/*! \fn void TwitPicView::resizeEvent( QResizeEvent *e )
    Reimplemented from QDialog, adjusts the photo preview when resizing its viewport.
*/
