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


#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class WelcomeDialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT
public:
    WelcomeDialog(QWidget *parent = 0);
    ~WelcomeDialog();

public slots:
    void confirmAccountAdded( bool success );

signals:
    void addAccount();

protected:
    void changeEvent(QEvent *e);

private slots:
    void proceed();

private:
    Ui::WelcomeDialog *m_ui;
};

#endif // WELCOMEDIALOG_H
