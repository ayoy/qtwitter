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


#ifndef AUTOTAGWIDGET_H
#define AUTOTAGWIDGET_H

#include <QWidget>
#include <QStringList>

namespace Ui {
    class AutoTagWidget;
}

class QStringListModel;

class AutoTagWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool active READ isActive WRITE setActive );
    Q_PROPERTY( QStringList tags READ tags WRITE setTags );

public:
    explicit AutoTagWidget( QWidget *parent = 0 );
    virtual ~AutoTagWidget();

    bool isActive() const;
    void setActive( bool active );
    QStringList tags() const;
    void setTags( const QStringList &tags );

public slots:
    void addTags();
    void deleteTag();

protected:
    void changeEvent( QEvent *event );
    bool eventFilter( QObject *watched, QEvent *event );

private slots:
    void updateAddButton();
    void updateDeleteButton();

private:
    Ui::AutoTagWidget *m_ui;
    QStringListModel *tagsModel;
};

#endif // AUTOTAGWIDGET_H
