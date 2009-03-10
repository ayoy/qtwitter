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


#ifndef TWITPICVIEW_H
#define TWITPICVIEW_H

#include <QtGui/QDialog>

namespace Ui {
  class TwitPicView;
}

class TwitPicView : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY(TwitPicView)
public:
  explicit TwitPicView(QWidget *parent = 0);
  virtual ~TwitPicView();

public slots:
  void showUploadProgress( int done, int total );
  void resetForm();
  void reject();

signals:
  void uploadPhoto( QString photoPath, QString status );
  void abortUpload();

protected:
  virtual void changeEvent( QEvent *e );
  void resizeEvent( QResizeEvent *e );

private slots:
  void sendUploadRequest();
  void setImagePath();
  void setImagePreview( const QString &path );

private:
  QString getHomeDir();
  QPixmap *pixmap;
  Ui::TwitPicView *m_ui;
};

#endif // TWITPICVIEW_H
