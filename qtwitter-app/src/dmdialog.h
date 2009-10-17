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


#ifndef DMDIALOG_H
#define DMDIALOG_H

#include <QDialog>
#include <twitterapi/twitterapi.h>

class QMovie;

namespace Ui {
    class DMDialog;
}

class DMDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(DMDialog);

public:
    explicit DMDialog( const QString &recipient, QWidget *parent = 0);
    virtual ~DMDialog();

public slots:
    void showResult( TwitterAPI::ErrorCode error );

protected:
    virtual void changeEvent( QEvent *e );

signals:
    void dmRequest( const QString &screenName, const QString &text );

private slots:
    void sendDM();
    void updateCharsLeft();
    void resetDisplay();

private:
    int charsLeft();
    QMovie *progress;
    QString serviceUrl;
    QString login;
    QString screenName;
    Ui::DMDialog *m_ui;
};

#endif // DMDIALOG_H
