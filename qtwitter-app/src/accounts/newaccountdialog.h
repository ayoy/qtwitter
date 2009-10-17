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


#ifndef NEWACCOUNTDIALOG_H
#define NEWACCOUNTDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class NewAccountDialog;
}

class NewAccountDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY( QString networkName READ networkName );
    Q_PROPERTY( QString serviceUrl READ serviceUrl );
    Q_PROPERTY( QString login READ login );
    Q_PROPERTY( QString password READ password );

public:
    NewAccountDialog( QWidget *parent = 0 );
    ~NewAccountDialog();

    QString networkName() const;
    QString serviceUrl() const;
    QString login() const;
    QString password() const;

protected:
    void changeEvent(QEvent *e);

private slots:
    void toggleEdits( int index );
    void checkFields();
    void shrink();

private:
    Ui::NewAccountDialog *m_ui;
};

#endif // NEWACCOUNTDIALOG_H
