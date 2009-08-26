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


#include "newaccountdialog.h"
#include "ui_newaccountdialog.h"
#include <account.h>
#include <QTimer>
#include <QPushButton>

NewAccountDialog::NewAccountDialog( QWidget *parent ) :
    QDialog( parent ),
    m_ui( new Ui::NewAccountDialog )
{
  m_ui->setupUi( this );

  foreach ( QString network, Account::networkNames() ) {
    if ( network != Account::NetworkTwitter &&
         network != Account::NetworkIdentica ) {
      m_ui->comboBox->addItem( network );
    }
  }
  m_ui->comboBox->addItem( tr( "Other laconi.ca" ) );


  connect( m_ui->nameEdit, SIGNAL(textChanged(QString)), SLOT(checkFields()) );
  connect( m_ui->urlEdit, SIGNAL(textChanged(QString)), SLOT(checkFields()) );
  connect( m_ui->loginEdit, SIGNAL(textChanged(QString)), SLOT(checkFields()) );
  connect( m_ui->passwordEdit, SIGNAL(textChanged(QString)), SLOT(checkFields()) );
  connect( m_ui->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(checkFields()) );

#ifndef OAUTH
  m_ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
#endif
  connect( m_ui->comboBox, SIGNAL(currentIndexChanged(int)), SLOT(toggleEdits(int)) );
  toggleEdits( 0 );
}

NewAccountDialog::~NewAccountDialog()
{
  delete m_ui;
}

void NewAccountDialog::checkFields()
{
  QPushButton *okButton = m_ui->buttonBox->button( QDialogButtonBox::Ok );
  bool enabled = false;
  switch ( m_ui->comboBox->currentIndex() ) {
  case 0: // Twitter
#ifndef OAUTH
    if ( !m_ui->loginEdit->text().isEmpty() &&
         !m_ui->passwordEdit->text().isEmpty() ) {
      enabled = true;
    } else {
      enabled = false;
    }
#else
    enabled = true;
#endif
    break;
  case 1: // Identi.ca
    if ( !m_ui->loginEdit->text().isEmpty() &&
         !m_ui->passwordEdit->text().isEmpty() ) {
      enabled = true;
    } else {
      enabled = false;
    }
    break;
  default:
    if ( !m_ui->nameEdit->text().isEmpty() &&
         !m_ui->urlEdit->text().isEmpty() &&
         !m_ui->loginEdit->text().isEmpty() &&
         !m_ui->passwordEdit->text().isEmpty() ) {
      enabled = true;
    } else {
      enabled = false;
    }
  }
  okButton->setEnabled( enabled );
}

QString NewAccountDialog::networkName() const
{
  if ( m_ui->comboBox->currentIndex() <= 1 ) {
    return m_ui->comboBox->currentText();
  }
  return m_ui->nameEdit->text();
}

QString NewAccountDialog::serviceUrl() const
{
  if ( m_ui->comboBox->currentIndex() <= 1 ) {
    return Account::networkUrl( m_ui->comboBox->currentText() );
  }
  QString url = m_ui->urlEdit->text();
  if ( url.endsWith( '/' ) ) {
    url.append( "api" );
  } else {
    url.append( "/api" );
  }
  if ( !url.startsWith( "http://" ) &&
       !url.startsWith( "https://" ) ) {
    return url.prepend( "http://" );
  }
  return url;
}

QString NewAccountDialog::login() const
{
  return m_ui->loginEdit->text();
}

QString NewAccountDialog::password() const
{
  return m_ui->passwordEdit->text();
}

void NewAccountDialog::toggleEdits( int index )
{
  bool enabled = true;

  switch ( index ) {
  case 0: // Twitter
  case 1: // Identi.ca
    m_ui->nameEdit->setVisible( false );
    m_ui->nameLabel->setVisible( false );
    m_ui->urlEdit->setVisible( false );
    m_ui->urlLabel->setVisible( false );
#ifdef OAUTH
    enabled = ( index != 0 ); // Twitter
#endif
    m_ui->loginEdit->setVisible( enabled );
    m_ui->loginLabel->setVisible( enabled );
    m_ui->passwordEdit->setVisible( enabled );
    m_ui->passwordLabel->setVisible( enabled );
    break;
  default:
    if ( index < m_ui->comboBox->count() - 1 ) {
      m_ui->nameEdit->setText( m_ui->comboBox->currentText() );
      m_ui->urlEdit->setText( Account::networkUrl(
          m_ui->comboBox->currentText() ).remove( QRegExp( "/api$", Qt::CaseInsensitive ) ) );
    } else if ( index == m_ui->comboBox->count() - 1 ) {
      m_ui->nameEdit->clear();
      m_ui->urlEdit->clear();
    }
    m_ui->nameEdit->setVisible( true );
    m_ui->nameLabel->setVisible( true );
    m_ui->urlEdit->setVisible( true );
    m_ui->urlLabel->setVisible( true );
    m_ui->loginEdit->setVisible( true );
    m_ui->loginLabel->setVisible( true );
    m_ui->passwordEdit->setVisible( true );
    m_ui->passwordLabel->setVisible( true );
  }
  QTimer::singleShot(0, this, SLOT(shrink()) );
}

void NewAccountDialog::shrink()
{
  resize( width(), 0 );
}

void NewAccountDialog::changeEvent(QEvent *e)
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
