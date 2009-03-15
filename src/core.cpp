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

#include <QSettings>
#include <QDesktopServices>
#include "ui_authdialog.h"
#include "ui_twitpicnewphoto.h"
#include "twitpicengine.h"

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    publicTimelineSync( false ),
    directMessagesSync( false ),
    switchUser( false ),
    authDialogOpen( false ),
    xmlGet( NULL ),
    xmlPost( NULL ),
    twitpicUpload( NULL ),
    timer( NULL ),
    statusesDone( false ),
    messagesDone( false )
{
}

Core::~Core() {}

void Core::applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages )
{
  bool a = setTimerInterval( msecs );
  bool b = setAuthData( user, password );
  bool c = setPublicTimelineSync( publicTimeline );
  bool d = setDirectMessagesSync( directMessages );
  if ( a || b || c || (!c && d) )
    get();
}

bool Core::isPublicTimelineSync()
{
  return publicTimelineSync;
}

bool Core::isDirectMessagesSync()
{
  return directMessagesSync;
}

bool Core::setTimerInterval( int msecs )
{
  bool initialization = !(bool) timer;
  if ( initialization ) {
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(get()) );
  }
  if ( timer->interval() != msecs ) {
    timer->setInterval( msecs );
    timer->start();
    if ( !initialization ) {
      return true;
    }
  }
  return false;
}

bool Core::setAuthData( const QString &user, const QString &password )
{
  switchUser = false;
  authData.setUser( user );
  authData.setPassword( password );
  if ( currentUser.isNull() ) {
    currentUser = user;
  } else if ( currentUser.compare( authData.user() ) ) {
    switchUser = true;
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  return switchUser;
}

bool Core::setPublicTimelineSync( bool b )
{
  if ( publicTimelineSync != b ) {
    publicTimelineSync = b;
    return true;
  }
  return false;
}

bool Core::setDirectMessagesSync( bool b )
{
  if ( directMessagesSync != b ) {
    directMessagesSync = b;
    if ( directMessagesSync == false ) {
      emit noDirectMessages();
    }
    return true;
  }
  return false;
}

#ifdef Q_WS_X11
void Core::setBrowserPath( const QString &path )
{
  browserPath = path;
}
#endif

void Core::forceGet()
{
  timer->start();
  get();
}

void Core::get()
{
  if ( publicTimelineSync ) {
    xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, this );
    xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", XmlDownload::Statuses );
  } else {
    if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
      if ( authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.user().isEmpty() ? QString() : authData.password() ) == Rejected ) {
        emit errorMessage( tr("Authentication is required to get your friends' updates.") );
        switchUser = false;
        return;
      }
    }
    xmlGet = new XmlDownload ( XmlDownload::RefreshStatuses, this );
    if ( publicTimelineSync ) {
      xmlGet->getContent( "http://twitter.com/statuses/public_timeline.xml", XmlDownload::Statuses );
    } else {
      xmlGet->getContent( "http://twitter.com/statuses/friends_timeline.xml", XmlDownload::Statuses );
      if ( directMessagesSync ) {
        xmlGet->getContent( "http://twitter.com/direct_messages.xml", XmlDownload::DirectMessages );
      }
    }
  }
  emit requestListRefresh( publicTimelineSync, switchUser );
  emit requestStarted();
  switchUser = false;
}

void Core::post( const QByteArray &status, int inReplyTo )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.user().isEmpty() ? QString() : authData.password() ) == Rejected ) {
      emit errorMessage( tr("Authentication is required to post updates.") );
      return;
    }
  }
  QByteArray request( "status=" );
  request.append( status );
  if ( inReplyTo != -1 ) {
    request.append( "&in_reply_to_status_id=" + QByteArray::number( inReplyTo ) );
  }
  request.append( "&source=qtwitter" );
  qDebug() << request;
  xmlPost = new XmlDownload( XmlDownload::Submit, this );
  xmlPost->postContent( "http://twitter.com/statuses/update.xml", request, XmlDownload::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  emit requestStarted();
  switchUser = false;
}

void Core::uploadPhoto( QString photoPath, QString status )
{
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.user().isEmpty() ? QString() : authData.password() ) == Rejected ) {
      emit errorMessage( tr("Authentication is required to upload photos to TwitPic.") );
      return;
    }
  }
  twitpicUpload = new TwitPicEngine( this );
  qDebug() << "uploading photo";
  twitpicUpload->postContent( authData, photoPath, status );
}

void Core::abortUploadPhoto()
{
  if ( twitpicUpload ) {
    twitpicUpload->abort();
    twitpicUpload->deleteLater();
    twitpicUpload = NULL;
  }
}

void Core::twitPicResponse( bool responseStatus, QString message, bool newStatus )
{
  emit twitPicResponseReceived();
  if ( !responseStatus ) {
    emit errorMessage( tr( "There was a problem uploading your photo:" ).append( " %1" ).arg( message ) );
    return;
  }
  if ( newStatus ) {
    forceGet();
  }
  twitpicUpload->deleteLater();
  twitpicUpload = NULL;
  QDialog dlg;
  Ui::TwitPicNewPhoto ui;
  ui.setupUi( &dlg );
  ui.textBrowser->setText( tr( "Photo available at:" ).append( " <a href=\"%1\">%1</a>" ).arg( message ) );
  dlg.exec();
}

void Core::destroyTweet( int id )
{
  qDebug() << "Tweet No." << id << "will be destroyed";
  if ( authData.user().isEmpty() || authData.password().isEmpty() ) {
    if ( authDataDialog( authData.user().isEmpty() ? QString() : authData.user(), authData.user().isEmpty() ? QString() : authData.password() ) == Rejected ) {
      emit errorMessage( tr("Authentication is required to delete updates.") );
      return;
    }
  }
  xmlPost = new XmlDownload( XmlDownload::Destroy, this );
  xmlPost->postContent( QString("http://twitter.com/statuses/destroy/%1.xml").arg( QString::number(id) ), QByteArray(), XmlDownload::Statuses );
  emit requestListRefresh( publicTimelineSync, switchUser );
  emit requestStarted();
  switchUser = false;
}

void Core::downloadImage( Entry *entry )
{
  if ( entry->getType() == Entry::Status ) {
    if ( imageCache.contains( entry->image() ) ) {
      if ( imageCache[ entry->image() ].isNull() ) {
        qDebug() << "not downloading";
      } else {
        emit setImageForUrl( entry->image(), imageCache[ entry->image() ] );
      }
      return;
    }
    QString host = QUrl( entry->image() ).host();
    if ( imageDownloader.contains( host ) ) {
      imageDownloader[host]->imageGet( entry );
      imageCache[ entry->image() ] = QImage();
      qDebug() << "setting null image";
      return;
    }
    ImageDownload *getter = new ImageDownload();
    imageDownloader[host] = getter;
    connect( getter, SIGNAL( errorMessage(QString) ), this, SIGNAL( errorMessage(QString) ) );
    connect( getter, SIGNAL(imageReadyForUrl(QString,QImage)), this, SLOT(setImageInHash(QString,QImage)) );
    getter->imageGet( entry );
    imageCache[ entry->image() ] = QImage();
    qDebug() << "setting null image" << imageCache[ entry->image() ].isNull();
  }
}

void Core::openBrowser( QUrl address )
{
  if ( address.isEmpty() )
    return;
#if defined Q_WS_MAC || defined Q_WS_WIN
  QDesktopServices::openUrl( address );
#elif defined Q_WS_X11
  QProcess *browser = new QProcess;
  if ( browserPath.isNull() ) {
    QDesktopServices::openUrl( address );
    return;
  }
  browser->start( browserPath + " " + address.toString() );
#endif
}

Core::AuthDialogState Core::authDataDialog( const QString &user, const QString &password)
{
  if ( authDialogOpen )
    return Accepted;
  emit resetUi();
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  ui.loginEdit->setText( ( user == QString() ) ? authData.user() : user );
  ui.loginEdit->selectAll();
  ui.passwordEdit->setText( password );
  dlg.adjustSize();
  authDialogOpen = true;
  if (dlg.exec() == QDialog::Accepted) {
    if ( ui.publicBox->isChecked() ) {
      publicTimelineSync = true;
      switchUser = false;
      emit switchToPublic();
      authDialogOpen = false;
      emit requestListRefresh( publicTimelineSync, switchUser );
      emit requestStarted();
      return SwitchToPublic;
    }
    publicTimelineSync = false;
    setAuthData( ui.loginEdit->text(), ui.passwordEdit->text() );
    emit authDataSet( authData );
    authDialogOpen = false;
    emit requestListRefresh( publicTimelineSync, switchUser );
    emit requestStarted();
    return Accepted;
  }
  qDebug() << "returning false";
  authDialogOpen = false;
  return Rejected;
}

const QAuthenticator& Core::getAuthData() const
{
  return authData;
}

void Core::setCookie( const QStringList newCookie )
{
  cookie = newCookie;
}

void Core::setFlag( XmlDownload::ContentRequested flag )
{
  switch ( flag ) {
    case XmlDownload::DirectMessages:
      messagesDone = true;
      break;
    case XmlDownload::Statuses:
    default:
      statusesDone = true;
  }
  emit resetUi();
  if ( publicTimelineSync ) {
    emit switchToPublic();
  }
  if ( statusesDone && ( publicTimelineSync || (!directMessagesSync ? true : messagesDone) || (xmlPost && !publicTimelineSync)  ) ) {
    emit timelineUpdated();
    emit authDataSet( authData );
    destroyXmlConnection();
    currentUser = authData.user();
    statusesDone = false;
    messagesDone = false;
  }
}

void Core::setImageInHash( const QString &url, QImage image )
{
  imageCache[ url ] = image;
  emit setImageForUrl( url, image );
}

void Core::newEntry( Entry *entry )
{
  if ( entry->login() == authData.user() ) {
    entry->setOwn( true );
  }
  emit addEntry( entry );
}

void Core::destroyXmlConnection()
{
  if ( xmlPost ) {
    qDebug() << "destroying xmlPost";
    xmlPost->deleteLater();
    xmlPost = NULL;
  }
  if ( xmlGet ) {
    qDebug() << "destroying xmlGet";
    xmlGet->deleteLater();
    xmlGet = NULL;
  }
}
