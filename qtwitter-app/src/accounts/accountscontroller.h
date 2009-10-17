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


#ifndef ACCOUNTSCONTROLLER_H
#define ACCOUNTSCONTROLLER_H

#include <QObject>

class QWidget;
class QModelIndex;
class AccountsModel;
class AccountsView;

namespace Ui {
    class Accounts;
}

class AccountsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool modified READ isModified WRITE setModified )

public:

    AccountsController( QWidget *widget, QObject *parent );
    virtual ~AccountsController();
    AccountsModel* getModel() const;
    void setModel( AccountsModel *model );

    bool isModified() const;
    void setModified( bool modified );

    void loadCustomNetworks();
    void loadAccounts();

public slots:
    void addAccount();
    void retranslateUi();

signals:
    void comboActive( bool isActive );
    void accountDialogClosed( bool success );

private slots:
    void updateAccounts( const QModelIndex &topLeft, const QModelIndex &bottomRight );
    void updateCheckBox( const QModelIndex &index );
    void togglePasswordStoring( int state );
    void showPasswordDisclaimer();
    void deleteAccount();

private:
    void setAccountEnabled( bool state );
    void setAccountDM( bool state );

    AccountsModel *model;
    AccountsView *view;
    Ui::Accounts *ui;

    bool modified;

    QWidget *widget;

    friend class AccountsDelegate;
};

#endif // ACCOUNTSCONTROLLER_H
