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


#ifndef TWITPICVIEW_H
#define TWITPICVIEW_H

#include <QtGui/QDialog>
#include <account.h>

namespace Ui {
    class TwitPicView;
}

class QStringList;

class TwitPicView : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(TwitPicView)

public:
    explicit TwitPicView(QWidget *parent = 0);
    virtual ~TwitPicView();

public slots:
    void setupAccounts( const QList<Account> &accounts );
    void setupAccounts( const QStringList &accounts );
    void showUploadProgress( qint64 done, qint64 total );
    void resetForm();
    void reject();

signals:
    void uploadPhoto( const QString &login, QString photoPath, QString status );
    void abortUpload();

protected:
    virtual void changeEvent( QEvent *e );
    void resizeEvent( QResizeEvent *e );

private slots:
    void sendUploadRequest();
    void setImagePath();
    void setImagePreview( const QString &path );

private:
    QString getHomeDir();
    QPixmap *pixmap;
    Ui::TwitPicView *m_ui;
};

#endif // TWITPICVIEW_H
