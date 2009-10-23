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


#ifndef URLSHORTENERWIDGET_H
#define URLSHORTENERWIDGET_H

#include <QWidget>
#include <QStringList>

namespace Ui {
    class UrlShortenerWidget;
}

class QStringListModel;

class UrlShortenerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool automatic READ isAutomatic WRITE setAutomatic );
    Q_PROPERTY( int currentIndex READ currentIndex WRITE setCurrentIndex );
    Q_PROPERTY( int currentShortener READ currentShortener );
    Q_PROPERTY( QString shortcut READ shortcut WRITE setShortcut );

public:
    explicit UrlShortenerWidget( QWidget *parent = 0 );
    virtual ~UrlShortenerWidget();

    bool isAutomatic() const;
    void setAutomatic( bool automatic );
    int currentIndex() const;
    void setCurrentIndex( int index );
    int currentShortener() const;
    QString shortcut() const;
    void setShortcut( const QString &shortcut );

    void setData( const QMap<QString,int> &data );

signals:
    void shortcutChanged( const QKeySequence &seq );

protected:
    void changeEvent( QEvent *event );
    bool eventFilter( QObject *watched, QEvent *event );

private:
    Ui::UrlShortenerWidget *m_ui;
    QStringListModel *shortenersModel;
};

#endif // URLSHORTENERWIDGET_H
