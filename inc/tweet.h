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


#ifndef TWEET_H
#define TWEET_H

#include <QtGui/QWidget>

namespace Ui {
    class Tweet;
}

class Tweet : public QWidget {
  Q_OBJECT
  Q_DISABLE_COPY( Tweet )
public:
  explicit Tweet( QWidget *parent = 0 );
  explicit Tweet( const QString &name, const QString &status, const QString &url, const QImage &icon, QWidget *parent = 0 );
  virtual ~Tweet();
  void resize( const QSize& );
  void resize( int w, int h );
  void setIcon( const QImage& );

  QString getUrlForIcon() const;

public slots:
  void adjustSize();

protected:
  virtual void changeEvent( QEvent *e );

private:
  QString urlForIcon;
  Ui::Tweet *m_ui;
};

#endif // TWEET_H
