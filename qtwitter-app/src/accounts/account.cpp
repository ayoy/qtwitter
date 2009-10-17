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


#include <QRegExp>
#include <QHash>
#include "account.h"
#include <configfile.h>

const QString Account::NetworkTwitter = "Twitter";
const QString Account::NetworkIdentica = "Identi.ca";
const QString Account::NetworkUrlTwitter = "https://twitter.com";
const QString Account::NetworkUrlIdentica = "https://identi.ca/api";

QHash<QString,QString> Account::networkNamesHash = QHash<QString,QString>();

extern ConfigFile settings;

Account::Account() {}

Account::Account( bool enabled, const QString &serviceUrl, const QString &login,
                  const QString &password, bool dm ) :
        m_enabled( enabled ),
        m_serviceUrl( serviceUrl ),
        m_login( login ),
        m_password( password ),
        m_dm( dm )
{
}

Account::Account( const Account &other ) :
        m_enabled( other.isEnabled() ),
        m_serviceUrl( other.serviceUrl() ),
        m_login( other.login() ),
        m_password( other.password() ),
        m_dm( other.dm() )
{
}

Account::~Account() {}

const QList<QString> Account::networkNames()
{
    return networkNamesHash.values();
}

QString Account::networkUrl( const QString &name )
{
    if ( networkNamesHash.values().contains( name ) ) {
        return networkNamesHash.key( name );
    }
    return QString();
}

QString Account::networkName( const QString &serviceUrl )
{
    if ( networkNamesHash.contains( serviceUrl ) ) {
        return networkNamesHash.value( serviceUrl );
    }
    return QString();
}

void Account::setNetworkName( const QString &serviceUrl, const QString &name )
{
    if ( name != NetworkTwitter &&
         name != NetworkIdentica ) {

        if ( networkNamesHash.keys().contains( name ) ) {
            networkNamesHash.remove( networkNamesHash.key( name ) );
        }
        if ( networkNamesHash.contains( serviceUrl ) ) {
            settings.remove( QString( "Services/%1" ).arg( networkNamesHash.value( serviceUrl ) ) );
        }
        if ( !networkNamesHash.contains( serviceUrl ) ) {
            networkNamesHash.insert( serviceUrl, name );
        }
        settings.setValue( QString( "Services/%1" ).arg( name ), serviceUrl );
        settings.sync();

    } else {
        if ( networkNamesHash.keys().contains( name ) ) {
            networkNamesHash.remove( networkNamesHash.key( name ) );
        }
        if ( !networkNamesHash.contains( serviceUrl ) ) {
            networkNamesHash.insert( serviceUrl, name );
        }
    }
}

void Account::removeNetwork( const QString &serviceUrl )
{
    if ( networkNamesHash.contains( serviceUrl ) ) {
        networkNamesHash.remove( serviceUrl );
        settings.remove( QString( "Services/%1" ).arg( networkNamesHash.value( serviceUrl ) ) );
        settings.sync();
    }
}


bool Account::isEnabled() const
{
    return m_enabled;
}

void Account::setEnabled( bool enabled )
{
    m_enabled = enabled;
}

QString Account::serviceUrl() const
{
    return m_serviceUrl;
}

void Account::setServiceUrl( const QString &serviceUrl )
{
    m_serviceUrl = serviceUrl;
}

QString Account::login() const
{
    return m_login;
}

void Account::setLogin( const QString &login )
{
    m_login = login;
}

QString Account::password() const
{
    return m_password;
}

void Account::setPassword( const QString &password )
{
    m_password = password;
}

bool Account::dm() const
{
    return m_dm;
}

void Account::setDM( bool dm )
{
    m_dm = dm;
}




QPair<QString,QString> Account::fromString( const QString &name )
{
    QRegExp rx( "(.+) @(.+)" );
    if ( rx.indexIn( name ) == -1 )
        return QPair<QString,QString>();
    return QPair<QString,QString>( rx.cap(2), rx.cap(1) );
}

QString Account::toString() const
{
    return QString( "%1 @ %2" ).arg( m_login, networkName( m_serviceUrl ) );
}

Account& Account::operator=( const Account &other )
{
    m_enabled = other.isEnabled();
    m_serviceUrl = other.serviceUrl();
    m_login = other.login();
    m_password = other.password();
    m_dm = other.dm();
    return *this;
}

bool Account::operator==( const Account &other ) const
{
    return ( m_enabled == other.isEnabled() &&
             m_serviceUrl == other.serviceUrl() &&
             m_login == other.login() &&
             m_password == other.password() &&
             m_dm == other.dm() );
}

bool Account::fuzzyCompare( const Account &other ) const
{
    return ( m_enabled == other.isEnabled() &&
             m_serviceUrl == other.serviceUrl() &&
             m_login == other.login() &&
             m_password == other.password() );
}

bool Account::operator<( const Account &other ) const
{
#if QT_VERSION >= 0x040500
    int compare = m_login.localeAwareCompare( other.login() );
#else
    int compare = m_login.compare( other.login() );
#endif
    if ( compare != 0 )
        return compare < 0;
    return networkName( m_serviceUrl ) < networkName( other.serviceUrl() );
}

QDataStream& operator<<( QDataStream & out, const Account &account )
{
    out << (qint8) account.isEnabled();
    // changed from qint8
    out << account.serviceUrl();
    out << account.login();
    out << account.password();
    out << (qint8) account.dm();
    return out;
}

QDataStream& operator>>( QDataStream & in, Account &account )
{
    qint8 en;
    // changed from qint8
    QString serviceUrl;
    QString login;
    QString password;
    qint8 dm;
    in >> en;
    in >> serviceUrl;
    in >> login;
    in >> password;
    in >> dm;
    account.setEnabled( (bool) en );
    account.setServiceUrl( serviceUrl );
    account.setLogin( login );
    account.setPassword( password );
    account.setDM( (bool) dm );
    return in;
}
