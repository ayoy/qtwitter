/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of      *
 *   the License, or (at your option) any later version.                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to                     *
 *   the Free Software Foundation, Inc.,                                   *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/


#ifndef TWITTERAPI_GLOBAL_H
#define TWITTERAPI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TWITTERAPI)
#  define TWITTERAPI_EXPORT Q_DECL_EXPORT
#else
#  define TWITTERAPI_EXPORT Q_DECL_IMPORT
#endif

namespace TwitterAPI {
  enum SocialNetwork {
    SOCIALNETWORK_TWITTER,
    SOCIALNETWORK_IDENTICA
  };

  /*!
    Set for sent requests, represents the activity performed by the specific request.
  */
  enum Role {
    ROLE_PUBLIC_TIMELINE = 101,
    ROLE_FRIENDS_TIMELINE,
    ROLE_DIRECT_MESSAGES,
    ROLE_POST_UPDATE,
    ROLE_DELETE_UPDATE,
    ROLE_POST_DM,
    ROLE_DELETE_DM,
    ROLE_FAVORITES_CREATE,
    ROLE_FAVORITES_DESTROY
  };

  enum ErrorCode {
    ERROR_NO_ERROR = 0,
    ERROR_DM_NOT_ALLOWED,
    ERROR_DM_USER_NOT_FOUND
  };

  /*!
    A constant used as a "login" for public timeline requests.
  */
  const QString PUBLIC_TIMELINE = "public timeline";

  const QString URL_IDENTICA = "http://identi.ca/api";
  const QString URL_TWITTER = "http://twitter.com";
}

#endif // TWITTERAPI_GLOBAL_H

/*!
  \var TwitterAPI::ROLE_PUBLIC_TIMELINE

  Indicates that the request is for downloading the public timeline.
*/

/*!
  \var TwitterAPI::ROLE_FRIENDS_TIMELINE

  Indicates that the request is for downloading the friends timeline.
*/

/*!
  \var TwitterAPI::ROLE_DIRECT_MESSAGES

  Indicates that the request is for downloading direct messages list.
*/

/*!
  \var TwitterAPI::ROLE_POST_UPDATE

  Indicates that the request is for posting a status update.
*/

/*!
  \var TwitterAPI::ROLE_DELETE_UPDATE

  Indicates that the request is for deleting a status update.
*/

/*!
  \var TwitterAPI::ROLE_POST_DM

  Indicates that the request is for posting a direct message.

  \note Not implemented yet.
*/

/*!
  \var TwitterAPI::ROLE_DELETE_DM

  Indicates that the request is for deleting a direct message.

  \note Not implemented yet.
*/
