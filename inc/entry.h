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

#include <QString>
#include <QObject>
#include <QDateTime>

class Entry : public QObject {

public:
  enum Type {
    Status,
    DirectMessage
  };

  Entry( Type entryType = Status, QObject *parent = 0 );
  Entry( int itemIndex, int id, const QString &text, const QString &name, const QString &login, const QString &image, const QString &homepage, QObject *parent = 0 );
  Entry( const Entry &right );

  Entry& operator=( const Entry &right );

  bool checkContents();
  void initialize( bool resetIndex = false );

  Type getType() const;
  int getIndex() const;
  bool isOwn() const;
  int id() const;
  QString name() const;
  QString login() const;
  QString homepage() const;
  QString image() const;
  QString text() const;

  void setIndex( int itemIndex );
  void setId( int newId );
  void setOwn( bool isOwn );
  void setName( const QString& newName );
  void setLogin( const QString& newLogin );
  void setHomepage( const QString& newHomepage );
  void setHasHomepage( bool );
  void setImage( const QString& newImage );
  void setText( const QString& newText );

private:
  Type type;
  int index;
  bool own;

  int userId;
  QString userText;
  QString userName;
  QString userLogin;
  QString userImage;
  QString userHomepage;
  bool hasHomepage;
  QDateTime userCreatedAt;
};

#endif //ENTRY_H
