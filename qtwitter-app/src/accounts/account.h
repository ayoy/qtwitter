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


#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QPair>
#include <QMetaType>
#include <twitterapi/twitterapi.h>
#include <QDataStream>

class Account
{
public:
    Account();
    Account( bool enabled, const QString &serviceUrl,
             const QString &login, const QString &password, bool dm);
    Account( const Account &other );
    ~Account();

    static const QList<QString> networkNames();
    static QString networkUrl( const QString &name );
    static QString networkName( const QString &serviceUrl );
    static void setNetworkName( const QString &serviceUrl, const QString &name );
    static void removeNetwork( const QString &serviceUrl );
    static const QString NetworkTwitter;
    static const QString NetworkIdentica;
    static const QString NetworkUrlTwitter;
    static const QString NetworkUrlIdentica;

    bool isEnabled() const;
    void setEnabled( bool enabled );
    QString serviceUrl() const;
    void setServiceUrl( const QString &serviceUrl );
    QString login() const;
    void setLogin( const QString &login );
    QString password() const;
    void setPassword( const QString &password );
    bool dm() const;
    void setDM( bool dm );

    static QPair<QString,QString> fromString( const QString &name );

    Account& operator=( const Account &other );
    bool operator==( const Account &other ) const;
    bool fuzzyCompare( const Account &other ) const;
    bool operator<( const Account &other ) const;
    QString toString() const;

private:
    // url/name
    static QHash<QString,QString> networkNamesHash;

    bool m_enabled;
    QString m_serviceUrl;
    QString m_login;
    QString m_password;
    bool m_dm;
};

QDataStream& operator<<( QDataStream &out, const Account &account );
QDataStream& operator>>( QDataStream &in, Account &account );

Q_DECLARE_METATYPE(Account)

#endif // ACCOUNT_H
