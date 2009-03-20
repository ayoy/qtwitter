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
#include "twitterapi.h"
#include "twitpicengine.h"

Core::Core( MainWindow *parent ) :
    QObject( parent ),
    switchUser( false ),
    authDialogOpen( false ),
    twitpicUpload( NULL ),
    timer( NULL )
{
  twitterapi = new TwitterAPI( this );
  connect( twitterapi, SIGNAL(addEntry(Entry*)), this, SIGNAL(addEntry(Entry*)) );
  connect( twitterapi, SIGNAL(addEntry(Entry*)), this, SLOT(downloadImage(Entry*)) );
  connect( twitterapi, SIGNAL(deleteEntry(int)), this, SIGNAL(deleteEntry(int)) );
  connect( twitterapi, SIGNAL(timelineUpdated()), this, SIGNAL(timelineUpdated()) );
  connect( twitterapi, SIGNAL(authDataSet(QAuthenticator)), this, SIGNAL(authDataSet(QAuthenticator)) );
  connect( twitterapi, SIGNAL(requestListRefresh(bool,bool)), this, SIGNAL(requestListRefresh(bool,bool)) );
  connect( twitterapi, SIGNAL(done()), this, SIGNAL(resetUi()) );
  connect( twitterapi, SIGNAL(unauthorized()), this, SLOT(slotUnauthorized()) );
  connect( twitterapi, SIGNAL(unauthorized(QByteArray,int)), this, SLOT(slotUnauthorized(QByteArray,int)) );
  connect( twitterapi, SIGNAL(unauthorized(int)), this, SLOT(slotUnauthorized(int)) );
  connect( twitterapi, SIGNAL(directMessagesSyncChanged(bool)), this, SIGNAL(directMessagesSyncChanged(bool)) );
  connect( twitterapi, SIGNAL(publicTimelineSyncChanged(bool)), this, SIGNAL(publicTimelineSyncChanged(bool)) );
}

Core::~Core() {}

void Core::applySettings( int msecs, const QString &user, const QString &password, bool publicTimeline, bool directMessages )
{
  bool a = setTimerInterval( msecs );
  bool b = twitterapi->setAuthData( user, password );
  bool c = twitterapi->setPublicTimelineSync( publicTimeline );
  bool d = twitterapi->setDirectMessagesSync( directMessages );
  if ( a || b || c || (!c && d) )
    get();
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
  Core::AuthDialogState state;
  while ( !twitterapi->get() ) {
    state = authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() );
    switch ( state ) {
    case Core::Rejected:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
      return;
    case Core::SwitchToPublic:
      twitterapi->setPublicTimelineSync( true );
      emit requestListRefresh( twitterapi->isPublicTimelineSync(), false );
      break;
    default:;
    }
  }
  emit requestStarted();
}

void Core::post( const QByteArray &status, int inReplyTo )
{
  twitterapi->post( status, inReplyTo );
  emit requestStarted();
}

void Core::uploadPhoto( QString photoPath, QString status )
{
  if ( twitterapi->getAuthData().user().isEmpty() || twitterapi->getAuthData().password().isEmpty() ) {
    if ( authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() ) == Rejected ) {
      emit errorMessage( tr("Authentication is required to upload photos to TwitPic.") );
      return;
    }
  }
  twitpicUpload = new TwitPicEngine( this );
  qDebug() << "uploading photo";
  twitpicUpload->postContent( twitterapi->getAuthData(), photoPath, status );
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
  twitterapi->destroyTweet( id );
  emit requestStarted();
}

void Core::downloadImage( Entry *entry )
{
  if ( entry->getType() == Entry::DirectMessage )
    return;

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
  connect( getter, SIGNAL(errorMessage(QString)), this, SIGNAL(errorMessage(QString)) );
  connect( getter, SIGNAL(imageReadyForUrl(QString,QImage)), this, SLOT(setImageInHash(QString,QImage)) );
  getter->imageGet( entry );
  imageCache[ entry->image() ] = QImage();
  qDebug() << "setting null image" << imageCache[ entry->image() ].isNull();
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

Core::AuthDialogState Core::authDataDialog( const QString &user, const QString &password )
{
  if ( authDialogOpen )
    return Accepted;
  emit resetUi();
  QDialog dlg;
  Ui::AuthDialog ui;
  ui.setupUi(&dlg);
  ui.loginEdit->setText( ( user == QString() ) ? twitterapi->getAuthData().user() : user );
  ui.loginEdit->selectAll();
  ui.passwordEdit->setText( password );
  dlg.adjustSize();
  authDialogOpen = true;
  if (dlg.exec() == QDialog::Accepted) {
    if ( ui.publicBox->isChecked() ) {
      authDialogOpen = false;
      twitterapi->setPublicTimelineSync( true );
      emit requestListRefresh( twitterapi->isPublicTimelineSync(), false );
      emit requestStarted();
      return SwitchToPublic;
    }
    twitterapi->setAuthData( ui.loginEdit->text(), ui.passwordEdit->text() );
    emit authDataSet( twitterapi->getAuthData() );
    authDialogOpen = false;
    twitterapi->setPublicTimelineSync( false );
    emit requestListRefresh( twitterapi->isPublicTimelineSync(), true );
    emit requestStarted();
    return Accepted;
  }
  qDebug() << "returning false";
  authDialogOpen = false;
  return Rejected;
}

void Core::setImageInHash( const QString &url, QImage image )
{
  imageCache[ url ] = image;
  emit setImageForUrl( url, image );
}

bool Core::retryAuthorizing( int role )
{
  Core::AuthDialogState state = authDataDialog( twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().user(), twitterapi->getAuthData().user().isEmpty() ? QString() : twitterapi->getAuthData().password() );
  switch ( state ) {
  case Core::Rejected:
    switch ( role ) {
    case TwitterAPI::Submit:
      emit errorMessage( tr( "Authentication is required to post updates." ) );
    case TwitterAPI::Destroy:
      emit errorMessage( tr( "Authentication is required to delete updates." ) );
    case TwitterAPI::Refresh:
      emit errorMessage( tr( "Authentication is required to get your friends' updates." ) );
    }
    twitterapi->abort();
    return false;
  case Core::SwitchToPublic:
    twitterapi->abort();
    twitterapi->setPublicTimelineSync( true );
  default:;
  }
  return true;
}

void Core::slotUnauthorized()
{
  if ( !retryAuthorizing( TwitterAPI::Refresh ) )
    return;
  twitterapi->get();
}

void Core::slotUnauthorized( const QByteArray &status, int inReplyToId )
{
  if ( !retryAuthorizing( TwitterAPI::Submit ) )
    return;
  twitterapi->post( status, inReplyToId );
}

void Core::slotUnauthorized( int destroyId )
{
  if ( !retryAuthorizing( TwitterAPI::Destroy ) )
    return;
  twitterapi->destroyTweet( destroyId );
}
