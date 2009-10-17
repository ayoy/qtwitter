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


#include "welcomedialog.h"
#include "ui_welcomedialog.h"

#include <QInputDialog>

#include "configfile.h"

extern ConfigFile settings;


WelcomeDialog::WelcomeDialog(QWidget *parent) :
        QDialog(parent),
        m_ui(new Ui::WelcomeDialog)
{
    m_ui->setupUi(this);
    m_ui->continueButton->hide();
    connect( m_ui->addAccountButton, SIGNAL(clicked()), this, SIGNAL(addAccount()) );
    connect( m_ui->continueButton, SIGNAL(clicked()), this, SLOT(proceed()) );
}

WelcomeDialog::~WelcomeDialog()
{
    delete m_ui;
}

void WelcomeDialog::confirmAccountAdded( bool success )
{
    if ( success ) {
        m_ui->infoLabel->setText( tr( "Account added successfully! You may add another or just start using program." ) );
        m_ui->addAccountButton->setText( tr( "Add one more" ) );
    } else {
        m_ui->infoLabel->setText( tr( "There was a problem adding your account. "
                                      "Please check your internet connection and try one more time." ) );
    }
    m_ui->addAccountButton->setEnabled( true );
    m_ui->continueButton->show();
}

void WelcomeDialog::proceed()
{
    m_ui->addAccountButton->hide();
    m_ui->infoLabel->setText( tr( "You can add more accounts at any time in Settings->Accounts." ) );
    m_ui->continueButton->setText( tr( "OK" ) );
    m_ui->continueButton->disconnect( this, SLOT(proceed()) );
    connect( m_ui->continueButton, SIGNAL(clicked()), this, SLOT(accept()) );
}

void WelcomeDialog::changeEvent(QEvent *e)
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
