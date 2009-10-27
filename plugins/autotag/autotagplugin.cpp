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


#include "autotagplugin.h"
#include "autotagwidget.h"
#include <QStringList>
#include <QVariant>
#include <QSettings>

AutoTagPlugin::AutoTagPlugin( QObject *parent ) :
        QObject( parent ),
        autoTagWidget( new AutoTagWidget )
{
}

AutoTagPlugin::~AutoTagPlugin()
{
    delete autoTagWidget;
    autoTagWidget = 0;
}

QString AutoTagPlugin::filterStatusBeforePosting( const QString &status )
{
    if ( !autoTagWidget->isActive() )
        return status;

    QStringList list = autoTagWidget->tags();
    if ( list.isEmpty() )
        return status;

    QString newStatus = status;
    foreach( QString tag, list ) {
        // Don't touch groups (!tag), already hashed tags (#tag), parts of urls ({/.:_-+%?&=@}tag)
        QRegExp rx( QString("(^|[^!#/\\.\\?\\:%&=@_-+])\\b(%1)\\b").arg(tag), Qt::CaseInsensitive );
        newStatus.replace( rx, "\\1#\\2" );
    }
    return newStatus;
}

QString AutoTagPlugin::tabName()
{
    return tr( "AutoTag" );
}

QWidget* AutoTagPlugin::settingsWidget()
{
    return autoTagWidget;
}

void AutoTagPlugin::saveConfig( QSettings *file )
{
    file->beginGroup( "AutoTag" );
    file->setValue( "enabled", autoTagWidget->isActive() );
    file->setValue( "tags", autoTagWidget->tags() );
    file->endGroup();
    file->sync();
}

void AutoTagPlugin::loadConfig( QSettings *file )
{
    file->beginGroup( "AutoTag" );
    autoTagWidget->setActive( file->value( "enabled", false ).toBool() );
    autoTagWidget->setTags( file->value( "tags", QStringList() ).toStringList() );
    file->endGroup();
    file->sync();
}

Q_EXPORT_PLUGIN2(AutoTag, AutoTagPlugin);
