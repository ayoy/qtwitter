/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef ENTRY_H
#define ENTRY_H

#include <QMetaType>
#include <QDateTime>
#include "twitterapi_global.h"

class QString;

struct TWITTERAPISHARED_EXPORT Entry
{
  enum Type {
    Status,
    DirectMessage
  };

  Entry( Entry::Type entryType = Entry::Status );

  void initialize();
  bool checkContents();

  Type type;
  bool isOwn;
  int id;
  QString text;
  QString originalText;
  QString name;
  QString login;
  QString image;
  QString homepage;
  bool hasHomepage;
  QDateTime timestamp;
};

Q_DECLARE_METATYPE(Entry)

#endif //ENTRY_H
