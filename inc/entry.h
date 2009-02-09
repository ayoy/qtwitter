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

class Entry {

private:
  QString userName;
  QString userImage;
  QString userText;
  
public:
  Entry();
  Entry(const QString &name, const QString &image, const QString &text);
  Entry(const Entry &right);

  bool checkContents();
  Entry& operator=( const Entry &right );
      
  QString name() const;
  QString image() const;
  QString text() const;
  
  void setName( const QString& newName );
  void setImage( const QString& newImage );
  void setText( const QString& newText );
};

#endif //ENTRY_H
