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
#include <twitterapi/twitterapi_global.h>

NewAccountDialog::NewAccountDialog( QWidget *parent ) :
    QDialog( parent ),
    m_ui(new Ui::NewAccountDialog)
{
  m_ui->setupUi( this );
#ifdef OAUTH
  connect( m_ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleEdits(int)) );
  toggleEdits( 0 );
#endif
}

NewAccountDialog::~NewAccountDialog()
{
  delete m_ui;
}

int NewAccountDialog::network() const
{
  return m_ui->comboBox->currentIndex();
}

QString NewAccountDialog::login() const
{
  return m_ui->loginEdit->text();
}

QString NewAccountDialog::password() const
{
  return m_ui->passwordEdit->text();
}

#ifdef OAUTH
void NewAccountDialog::toggleEdits( int index )
{
  bool enabled = ( index != TwitterAPI::SOCIALNETWORK_TWITTER );
  m_ui->loginEdit->setEnabled( enabled );
  m_ui->loginLabel->setEnabled( enabled );
  m_ui->passwordEdit->setEnabled( enabled );
  m_ui->passwordLabel->setEnabled( enabled );
}
#endif

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
