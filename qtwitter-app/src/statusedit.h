/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#ifndef STATUSEDIT_H
#define STATUSEDIT_H

#include <QLineEdit>

class QEvent;
class QFocusEvent;

class StatusFilter : public QObject
{
  Q_OBJECT

public:
  StatusFilter( QObject *parent = 0 );

signals:
  void enterPressed();
  void escPressed();
  void shortenUrlPressed();

protected:
  bool eventFilter( QObject *dist, QEvent *event );
};


class StatusEdit : public QLineEdit
{
  Q_OBJECT
public:
  static const int STATUS_MAX_LENGTH;

  StatusEdit( QWidget * parent = 0 );

  void focusInEvent( QFocusEvent * event );
  void focusOutEvent( QFocusEvent * event );
  void initialize();
  bool isStatusClean() const;
  int getInReplyTo() const;
  QString getSelectedUrl() const;
  int charsLeft() const;

public slots:
  void cancelEditing();
  void addReplyString( const QString &name, quint64 inReplyTo );
  void addRetweetString( QString message );
  void shortenUrl();

signals:
  void errorMessage( const QString &message );
  void shortenUrl( const QString &url );

private:
  bool statusClean;
  quint64 inReplyToId;
  QString selectedUrl;

};

#endif //STATUSEDIT_H
