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


#include "core.h"

#include "ui_authdialog.h"
#include <QSettings>

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    downloadPublicTimeline( false ),
    showingDialog( false ),
    xmlGet( NULL ),
    xmlPost( NULL ),
    timer( NULL )
{
  connect( this, SIGNAL(xmlConnectionIdle()), SLOT(destroyXmlConnection()) );
}

Core::~Core() {}

void Core::setTimerInterval( int msecs )
{
  bool initialization = !(bool) timer;
  if ( !timer ) {
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(get()) );
  }
  if ( timer->interval() != msecs ) {
    timer->setInterval( msecs );
    timer->start();
  }
  if ( !initialization ) {
    get();
  }
}

void Core::forceGet()
{
  timer->start();
  get();
}

void Core::downloadOneImage( Entry *entry ) {
  if ( entry->getType() == Entry::Status ) {
    if ( imagesHash.contains( entry->image() ) ) {
      if ( imagesHash[ entry->image() ].isNull() ) {
        qDebug() << "not downloading";
      } else {
        emit setImageForUrl( entry->image(), imagesHash[ entry->image() ] );
      }
      return;
    }
    QString host = QUrl( entry->image() ).host();
    if ( imagesGetter.contains( host ) ) {
      imagesGetter[host]->imgGet( entry );
      imagesHash[ entry->image() ] = QImage();
      qDebug() << "setting null image";
      return;
    }
    ImageDownload *getter = new ImageDownload();
    imagesGetter[host] = getter;
    connect( getter, SIGNAL( errorMessage( const QString& ) ), this, SIGNAL( errorMessage( const QString& ) ) );
    connect( getter, SIGNAL(imageReadyForUrl(QString,QImage)), this, SLOT(setImageInHash(QString,QImage)) );
    getter->imgGet( entry );
    imagesHash[ entry->image() ] = QImage();
    qDebug() << "setting null image" << imagesHash[ entry->image() ].isNull();
  }
}

void Core::setImageInHash( const QString &url, QImage image ) {
  qDebug() << "assigning image to url:" << url;
  imagesHash[ url ] = image;
  emit setImageForUrl( url, image );
}

void Core::setDownloadPublicTimeline( bool b ) {
  if ( downloadPublicTimeline != b ) {
    downloadPublicTimeline = b;
    get();
  }
}

bool Core::downloadsPublicTimeline() {
  return downloadPublicTimeline;
}

void Core::newEntry( Entry *entry )
{
  if ( entry->login() == authData.user() ) {
    entry->setOwn( true );
  }
  emit addOneEntry( entry );
}

void Core::destroyTweet( int id )
{
  qDebug() << "Tweet No." << id << "will be destroyed";
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( !authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.password().isEmpty() ? QString() : authData.password() ) ) {
      emit errorMessage( tr("Authentication is required to post updates") );
      return;
    }
  }
  xmlPost = new XmlDownload( XmlDownload::Destroy, this );

  xmlPost->postContent( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ), QByteArray(), XmlDownload::Statuses );
}

void Core::get() {
  emit requestListRefresh();
  if ( downloadPublicTimeline ) {
     xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, this );
     xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", XmlDownload::Statuses );
   } else {
     if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
       if ( !authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.password().isEmpty() ? QString() : authData.password() ) ) {
         emit errorMessage( tr("Authentication is required to get your friends' updates") );
         return;
       }
     }
     if ( downloadPublicTimeline ) {
       xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, this );
       xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", XmlDownload::Statuses );
     } else {
       qDebug() << "creating XmlDownload";
       xmlGet = new XmlDownload ( XmlDownload::RefreshAll, this );
       xmlGet->getContent( "http://twitter.com/statuses/friends_timeline.xml", XmlDownload::Statuses );
       xmlGet->getContent( "http://twitter.com/direct_messages.xml", XmlDownload::DirectMessages );
    }
  }
}

void Core::post( const QByteArray &status ) {
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( !authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.password().isEmpty() ? QString() : authData.password() ) ) {
      emit errorMessage( tr("Authentication is required to post updates") );
      return;
    }
  }
  QByteArray request( "status=" );
  request.append( status );
  request.append( "&source=qtwitter" );
  qDebug() << request;
  xmlPost = new XmlDownload( XmlDownload::Submit, this );
  xmlPost->postContent( "http://twitter.com/statuses/update.xml", request, XmlDownload::Statuses );
}

void Core::destroyXmlConnection() {
  if ( xmlPost ) {
    qDebug() << "destroying xmlPost";
    delete xmlPost;
    xmlPost = NULL;
  }
  if ( xmlGet ) {
    qDebug() << "destroying xmlGet";
    delete xmlGet;
    xmlGet = NULL;
  }
}

void Core::storeCookie( const QStringList newCookie ) {
  cookie = newCookie;
}

bool Core::authDataDialog( const QString &name, const QString &password) {
  if ( showingDialog )
    return true;
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  ui.loginEdit->setText( name );
  ui.passwordEdit->setText( password );
  dlg.adjustSize();
  showingDialog = true;
  if (dlg.exec() == QDialog::Accepted) {
    if ( ui.publicBox->isChecked() ) {
      downloadPublicTimeline = true;
      emit switchToPublic();
    } else {
      authData.setUser( ui.loginEdit->text() );
      authData.setPassword( ui.passwordEdit->text() );
      emit authDataSet( authData );
    }
    showingDialog = false;
    return true;
  }
  showingDialog = false;
  return false;
}

void Core::setAuthData( const QString &username, const QString &password ) {
  bool refreshTweets = false;
  if ( authData.user().compare( username ) ) {
    authData.setUser( username );
    refreshTweets = true;
  }
  if ( authData.password().compare( password ) ) {
    authData.setPassword( password );
    refreshTweets = true;
  }
  if ( refreshTweets ) {
    get();
  }
}

const QAuthenticator& Core::getAuthData() const
{
  return authData;
}

#ifdef Q_WS_X11
void Core::setBrowserPath( const QString &path )
{
  browserPath = path;
}
#endif

void Core::openBrowser( QString address )
{
  if ( address.isNull() ) {
    address = "http://twitter.com/home" ;
  }
  QProcess *browser = new QProcess;
#ifdef Q_WS_MAC
  browser->start( "/usr/bin/open " + address );
#elif defined Q_WS_X11
  if ( browserPath.isEmpty() ) {
    emit errorMessage( tr( "Browser path is not defined. Specify it in Settings->Network section." ) );
    return;
  }
  browser->start( browserPath + " " + address );
#elif defined Q_WS_WIN
  QSettings settings( "HKEY_CLASSES_ROOT\\http\\shell\\open\\command", QSettings::NativeFormat );
  browser->start( settings.value( "Default" ).toString() + " " + address );
#endif
}
