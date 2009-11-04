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


#include "configfile.h"

#include <account.h>
#include <twitterapi/twitterapi_global.h>

#include <QFileInfo>
#include <QStringList>

AppVersion::AppVersion( uint maj, uint min, uint pat ) :
        majorVer( maj ),
        minorVer( min ),
        patchVer( pat )
{
}

AppVersion::AppVersion( const QString &version )
{
    fromString( version );
}

QString AppVersion::toString() const
{
    return QString( "%1.%2.%3" ).arg( QString::number(majorVer),
                                      QString::number(minorVer),
                                      QString::number(patchVer) );
}

void AppVersion::fromString( const QString &version )
{
    QStringList parts = version.split( "." );
    if ( parts.size() == 3 ) {
        majorVer = parts.at(0).toUInt();
        minorVer = parts.at(1).toUInt();
        patchVer = parts.at(2).toUInt();
    }
}

bool AppVersion::operator ==( const AppVersion &other ) const
{
    return majorVer == other.majorVer && minorVer == other.minorVer && patchVer == other.patchVer;
}

bool AppVersion::operator !=( const AppVersion &other ) const
{
    return !( *this == other );
}

bool AppVersion::operator >( const AppVersion &other ) const
{
    if ( majorVer != other.majorVer ) {
        return majorVer > other.majorVer;
    } else if ( minorVer != other.minorVer ) {
        return minorVer > other.minorVer;
    } else {
        return patchVer > other.patchVer;
    }
}

bool AppVersion::operator <( const AppVersion &other ) const
{
    if ( *this != other ) {
        return !( *this > other );
    } else {
        return false;
    }
}

bool AppVersion::operator >=( const AppVersion &other ) const
{
    return ( *this == other ) || ( *this > other );
}

bool AppVersion::operator <=( const AppVersion &other ) const
{
    return ( *this == other ) || ( *this < other );
}

const QString ConfigFile::APP_VERSION = "0.10.0";
const QString ConfigFile::COMPAT_SETTINGS_APP_VERSION = "0.9.0";


ConfigFile settings;


ConfigFile::ConfigFile():
#if defined Q_WS_MAC
        QSettings( QSettings::defaultFormat(), QSettings::UserScope, "ayoy.net", "qTwitter" )
#elif defined Q_WS_WIN
        QSettings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" )
#else
        QSettings( QSettings::defaultFormat(), QSettings::UserScope, "ayoy", "qTwitter" )
#endif
{
    if ( QFileInfo( fileName() ).exists() ) {
        if ( contains( "FIRSTRUN" ) ) {
            remove( "FIRSTRUN" );
        }
        remove( "OAuth" );
        QString ver = value( "General/version", QString() ).toString();
        if ( AppVersion( ver ) < AppVersion( COMPAT_SETTINGS_APP_VERSION ) ) {
            beginGroup( "Accounts" );
            remove("");
            endGroup();
        }
        if ( AppVersion( ver ) < AppVersion( APP_VERSION ) ) {
            fixForSsl();
        }
    } else {
        setValue( "FIRSTRUN", "ever" );
    }
    setValue( "General/version", ConfigFile::APP_VERSION );
    sync();
}

QString ConfigFile::pwHash( const QString &text )
{
    QString newText = text;
    for (unsigned int i = 0, textLength = text.length(); i < textLength; ++i)
        newText[i] = QChar(text[i].unicode() ^ i ^ 1);
    return newText;
}

void ConfigFile::addAccount( int id, const Account &account )
{
    beginGroup( QString( "Accounts/%1" ).arg( id ) );
    setValue( "enabled", account.isEnabled() );
    setValue( "service", account.serviceUrl() );
    setValue( "login", account.login() );
    setValue( "password", pwHash( account.password() ) );
    setValue( "directmsgs", account.dm() );
    endGroup();
    sync();
}

void ConfigFile::deleteAccount( int id, int rowCount )
{
    if ( id < rowCount ) {
        beginGroup( "Accounts" );
        for (int i = id; i < rowCount - 1; i++ ) {
            setValue( QString( "%1/enabled" ).arg(i), value( QString( "%1/enabled" ).arg(i+1) ) );
            setValue( QString( "%1/service" ).arg(i), value( QString( "%1/service" ).arg(i+1) ) );
            setValue( QString( "%1/login" ).arg(i), value( QString( "%1/login" ).arg(i+1) ) );
            setValue( QString( "%1/password" ).arg(i), value( QString( "%1/password" ).arg(i+1) ) );
            setValue( QString( "%1/directmsgs" ).arg(i), value( QString( "%1/directmsgs" ).arg(i+1) ) );
        }
        remove( QString::number( rowCount - 1) );
        endGroup();
        sync();
    }
}

int ConfigFile::accountsCount() const
{
    int count = 0;

    for( int i = 0;; ++i ) {
        if ( contains( QString( "Accounts/%1/enabled" ).arg(i) ) ) {
            count++;
        } else {
            break;
        }
    }

    return count;
}

void ConfigFile::fixForSsl()
{
    beginGroup( "Accounts" );

    int accountsCount = childGroups().count();
    for ( int i = 0; i < accountsCount; i++ ) {
        QString name = value( QString( "%1/service" ).arg(i) ).toString();
        if ( !name.isNull() &&
             ( name == "http://twitter.com" || name == "http://identi.ca/api" ) ) {
            name.replace( QRegExp( "^http" ), "https" );
        }
        setValue( QString( "%1/service" ).arg(i), name );
    }

    endGroup();
    sync();
}
