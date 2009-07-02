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

const QString ConfigFile::APP_VERSION = "0.7.1";


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
#else
  setValue( "OAuth", false );
#endif
  if ( QFileInfo( fileName() ).exists() ) {
    if ( contains( "FIRSTRUN" ) ) {
      remove( "FIRSTRUN" );
    }
    if ( value( "General/version", QString() ).toString() == "0.6.0" ) {
      convertSettingsToZeroSeven();
#ifdef OAUTH
      setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
      setValue( "OAuth", true );
#endif
    } else if ( value( "General/version", QString() ).toString().isNull() ) {
      convertSettingsToZeroSix();
      convertSettingsToZeroSeven();
#ifdef OAUTH
      setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
      setValue( "OAuth", true );
#endif
    } else if ( value( "General/version", QString() ).toString() != ConfigFile::APP_VERSION ) {
      setValue( "General/version", ConfigFile::APP_VERSION );
#ifdef OAUTH
      setValue( "FIRSTRUN", ConfigFile::APP_VERSION );
      setValue( "OAuth", true );
#endif
    }
  } else {
    setValue( "General/version", ConfigFile::APP_VERSION );
    setValue( "FIRSTRUN", "ever" );
  }
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
  settings.beginGroup( QString( "Accounts/%1" ).arg( id ) );
  settings.setValue( "enabled", account.isEnabled );
  settings.setValue( "service", account.network );
  settings.setValue( "login", account.login );
  settings.setValue( "password", pwHash( account.password ) );
  settings.setValue( "directmsgs", account.directMessages );
  settings.endGroup();
}

void ConfigFile::deleteAccount( int id, int rowCount )
{
  beginGroup( "Accounts" );
  if ( id < rowCount ) {
    for (int i = id; i < rowCount - 1; i++ ) {
      setValue( QString( "%1/enabled" ).arg(i), value( QString( "%1/enabled" ).arg(i+1) ) );
      setValue( QString( "%1/service" ).arg(i), value( QString( "%1/service" ).arg(i+1) ) );
      setValue( QString( "%1/login" ).arg(i), value( QString( "%1/login" ).arg(i+1) ) );
      setValue( QString( "%1/password" ).arg(i), value( QString( "%1/password" ).arg(i+1) ) );
      setValue( QString( "%1/directmsgs" ).arg(i), value( QString( "%1/directmsgs" ).arg(i+1) ) );
    }
  }
  remove( QString::number( rowCount - 1) );
  endGroup();
}

#ifdef OAUTH
void ConfigFile::removeOldTwitterAccounts()
{
  int count = 0;

  for( int i = 0;; ++i ) {
    if ( contains( QString( "Accounts/%1/enabled" ).arg(i) ) ) {
      count++;
    } else {
      break;
    }
  }

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
}

void ConfigFile::convertSettingsToZeroSeven()
{
  setValue( "General/version", ConfigFile::APP_VERSION );

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
}
