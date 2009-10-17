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


#include "statusedit.h"
#include "twitpictextedit.h"
#include "dmdialog.h"
#include "ui_dmdialog.h"
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QMovie>

DMDialog::DMDialog( const QString &screenName, QWidget *parent) :
        QDialog(parent),
        progress( new QMovie( ":/icons/progress.gif", "", this ) ),
        screenName( screenName ),
        m_ui(new Ui::DMDialog)
{
    m_ui->setupUi(this);

    m_ui->messageTextEdit->setAllowEnters( true );

    QFont charsLeftFont = m_ui->charsLeftLabel->font();
    charsLeftFont.setPointSize( charsLeftFont.pointSize() - 2 );
    m_ui->charsLeftLabel->setFont( charsLeftFont );
    m_ui->sendingLabel->setFont( charsLeftFont );
    m_ui->progressLabel->resize( m_ui->progressLabel->width(), m_ui->sendingLabel->height() );

    m_ui->charsLeftLabel->setText( tr( "%n characters left", "", charsLeft() ) );
    m_ui->sendingLabel->hide();
    m_ui->progressLabel->hide();
    m_ui->progressLabel->setMovie( progress );

    setWindowTitle( tr( "Direct message %1" ).arg( screenName ) );

    connect( m_ui->messageTextEdit, SIGNAL(textChanged()), this, SLOT(updateCharsLeft()) );
    connect( m_ui->sendButton, SIGNAL(clicked()), this, SLOT(sendDM()) );

    adjustSize();
}

DMDialog::~DMDialog()
{
    delete m_ui;
    m_ui = 0;
}

void DMDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DMDialog::updateCharsLeft()
{
    QString text;
    QPalette palette( m_ui->charsLeftLabel->palette() );
    int length = charsLeft();

    if ( length < 0 ) {
        palette.setColor( QPalette::Foreground, Qt::red );
        if ( length == -1 )
            text = tr( "%n character over the limit", "", length * -1 );
        else
            text = tr( "%n characters over the limit", "", length * -1 );
    } else {
        palette.setColor( QPalette::Foreground, Qt::black );
        if ( length == 1 )
            text = tr( "%n character left", "", length );
        else
            text = tr( "%n characters left", "", length );
    }

    m_ui->charsLeftLabel->setText( text );
    m_ui->charsLeftLabel->setPalette( palette );
}

void DMDialog::sendDM()
{
    if( charsLeft() < 0 ) {
        QMessageBox *messageBox = new QMessageBox( QMessageBox::Warning, tr( "Message too long" ), tr( "Your message is too long." ) );
        QPushButton *accept = messageBox->addButton( tr( "&Truncate" ), QMessageBox::AcceptRole );
        QPushButton *reject = messageBox->addButton( tr( "&Edit" ), QMessageBox::RejectRole );
        messageBox->setInformativeText( tr( "You can still post it like this, but it will be truncated." ) );
        messageBox->setDefaultButton( accept );
        messageBox->setEscapeButton( reject );
        messageBox->exec();
        if ( messageBox->clickedButton() == reject )
            return;
        messageBox->deleteLater();
    }

    emit dmRequest( screenName, m_ui->messageTextEdit->toPlainText() );

    progress->start();
    m_ui->sendingLabel->setText( tr( "Sending..." ) );
    m_ui->charsLeftLabel->hide();
    m_ui->sendingLabel->show();
    m_ui->progressLabel->show();
    m_ui->resetButton->setEnabled( false );
    m_ui->sendButton->setEnabled( false );
    m_ui->messageTextEdit->setEnabled( false );
    m_ui->closeButton->setFocus();
}

void DMDialog::showResult( TwitterAPI::ErrorCode error )
{
    progress->stop();
    m_ui->progressLabel->hide();
    if ( error == TwitterAPI::ERROR_NO_ERROR ) {
        m_ui->sendingLabel->setText( tr( "Message sent!" ) );
        QTimer::singleShot( 2000, this, SLOT(resetDisplay()) );
    } else {
        m_ui->sendingLabel->setText( tr( "Message sending failed. Note that you cannot send a message "
                                         "to a user that doesn't follow you." ) );
    }
}

void DMDialog::resetDisplay()
{
    m_ui->charsLeftLabel->show();
    m_ui->sendingLabel->hide();
    m_ui->progressLabel->hide();
    m_ui->resetButton->setEnabled( true );
    m_ui->sendButton->setEnabled( true );
    m_ui->messageTextEdit->setEnabled( true );
    m_ui->messageTextEdit->setFocus();
}

int DMDialog::charsLeft()
{
    return StatusEdit::STATUS_MAX_LENGTH - m_ui->messageTextEdit->toPlainText().length();
}
