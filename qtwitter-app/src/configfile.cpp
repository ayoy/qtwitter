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

const QString ConfigFile::APP_VERSION = "0.8.9999";
const QString ConfigFile::FIRST_OAUTH_APP_VERSION = "0.8.0";


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
#ifdef OAUTH
  if ( value( "OAuth", true ) == false ) {
    setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
  }
  if ( QFileInfo( fileName() ).exists() ) {
    if ( contains( "FIRSTRUN" ) ) {
      remove( "FIRSTRUN" );
    }
    QString ver = value( "General/version", QString() ).toString();
    if ( AppVersion( ver ) == AppVersion( "0.6.0" ) ) {
      convertSettingsToZeroSeven();
      convertSettingsToZeroNine();
      setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
      setValue( "OAuth", true );
    } else if ( ver.isNull() ) {
      convertSettingsToZeroSix();
      convertSettingsToZeroSeven();
      convertSettingsToZeroNine();
      setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
      setValue( "OAuth", true );
    } else if ( AppVersion( ver ) >= AppVersion( "0.7.0" ) &&
                AppVersion( ver ) < AppVersion( APP_VERSION ) ) {
      convertSettingsToZeroNine();
      if ( !value( "OAuth", false ).toBool() ) {
        setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
        setValue( "OAuth", true );
      }
    } else if ( AppVersion( value( "General/version", QString() ).toString() ) != AppVersion( ConfigFile::APP_VERSION ) ) {
      setValue( "General/version", ConfigFile::APP_VERSION );
      if ( !value( "OAuth", false ).toBool() ) {
        setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
        setValue( "OAuth", true );
      }
    }
  } else {
    setValue( "General/version", ConfigFile::APP_VERSION );
    setValue( "FIRSTRUN", "ever" );
    setValue( "OAuth", true );
  }
#else
  setValue( "OAuth", false );
  if ( QFileInfo( fileName() ).exists() ) {
    if ( contains( "FIRSTRUN" ) ) {
      remove( "FIRSTRUN" );
    }
    if ( AppVersion( value( "General/version", QString() ).toString() ) == AppVersion( "0.6.0" ) ) {
      convertSettingsToZeroSeven();
    } else if ( value( "General/version", QString() ).toString().isNull() ) {
      convertSettingsToZeroSix();
      convertSettingsToZeroSeven();
    } else if ( AppVersion( value( "General/version", QString() ).toString() ) >= AppVersion( "0.7.0" ) ) {
      convertSettingsToZeroNine();
    } else if ( AppVersion( value( "General/version", QString() ).toString() ) != AppVersion( ConfigFile::APP_VERSION ) ) {
      setValue( "General/version", ConfigFile::APP_VERSION );
    }
  } else {
    setValue( "General/version", ConfigFile::APP_VERSION );
    setValue( "FIRSTRUN", "ever" );
  }
#endif
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

#ifdef OAUTH
void ConfigFile::removeOldTwitterAccounts()
{
  int count = accountsCount();

  for( int i = 0; i < count; ++i ) {
    if ( value( QString( "Accounts/%1/service" ).arg(i), TwitterAPI::SOCIALNETWORK_IDENTICA ) ==
         TwitterAPI::SOCIALNETWORK_TWITTER ) {
      deleteAccount( i, count );
      count--;
      i--;
    }
  }
  sync();
}
#endif

void ConfigFile::convertSettingsToZeroSix()
{
  setValue( "General/version", "0.6.0" );
  if ( contains( "General/username" ) ) {
    setValue( "TwitterAccounts/0/enabled", true );
    setValue( "TwitterAccounts/0/service", TwitterAPI::SOCIALNETWORK_TWITTER );
    setValue( "TwitterAccounts/0/login", value( "General/username", "<empty>" ).toString() );
    setValue( "TwitterAccounts/0/password", value( "General/password", "" ).toString() );
    setValue( "TwitterAccounts/0/directmsgs", value( "General/directMessages", false ).toBool() );
  }
  if ( value( "General/timeline", false ).toBool() ) {
    setValue( "TwitterAccounts/currentModel", 1 );
  }
  remove( "TwitterAccounts/publicTimeline" );
  remove( "General/username" );
  remove( "General/password" );
  remove( "General/directMessages" );
  remove( "General/timeline" );
  sync();
}

void ConfigFile::convertSettingsToZeroSeven()
{
  setValue( "General/version", "0.7.0" );

  QString id;
  for( int i = 0;; ++i ) {
    id = QString::number(i);
    if ( contains( QString( "TwitterAccounts/%1/enabled" ).arg(id) ) ) {
      setValue( QString( "Accounts/%1/enabled" ).arg(id),
                value( QString( "TwitterAccounts/%1/enabled" ).arg(id) ).toBool() );
      setValue( QString( "Accounts/%1/service" ).arg(id),
                value( QString( "TwitterAccounts/%1/service" ).arg(id) ).toInt() );
      setValue( QString( "Accounts/%1/login" ).arg(id),
                value( QString( "TwitterAccounts/%1/login" ).arg(id) ).toString() );
      setValue( QString( "Accounts/%1/password" ).arg(id),
                value( QString( "TwitterAccounts/%1/password" ).arg(id) ).toString() );
      setValue( QString( "Accounts/%1/directmsgs" ).arg(id),
                value( QString( "TwitterAccounts/%1/directmsgs" ).arg(id) ).toString() );

      remove( QString( "TwitterAccounts/%1" ).arg(id) );
    } else
      break;
  }

  setValue( "Accounts/visibleAccount", value( "TwitterAccounts/currentModel" ).toInt() );
  setValue( "Appearance/color scheme", value( "Appearance/color scheme").toInt() - 1 );
  remove( "TwitterAccounts/currentModel" );
  sync();
}

void ConfigFile::convertSettingsToZeroNine()
{
  setValue( "General/version", ConfigFile::APP_VERSION );

  QString id;
  for( int i = 0;; ++i ) {
    id = QString::number(i);
    if ( contains( QString( "Accounts/%1/service" ).arg(id) ) ) {
      int network = value( QString( "Accounts/%1/service" ).arg(id), 0 ).toInt();
      switch ( network ) {
      case 0:
        setValue( QString( "Accounts/%1/service" ).arg(id), "http://twitter.com" );
        break;
      case 1:
        setValue( QString( "Accounts/%1/service" ).arg(id), "http://identi.ca/api" );
        break;
      }
    } else
      break;
  }
  sync();
}
