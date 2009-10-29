/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#include "statuslist.h"
#include "statuslist_p.h"

#include <QDebug>
#include <QPixmap>

#include <account.h>
#include <oauthwizard.h>
#include "imagedownload.h"
#include "core.h"
#include "dmdialog.h"
#include "qtwitterapp.h"

QDataStream& operator<<( QDataStream & out, const Status &status )
{
    out << status.entry;
    out << status.image;
    out << status.state;
    return out;
}

QDataStream& operator>>( QDataStream & in, Status &status )
{
    int state;
    in >> status.entry;
    in >> status.image;
    in >> state;
    status.state = (StatusModel::StatusState) state;
    return in;
}

const int StatusListPrivate::publicMaxCount = 20;
int StatusListPrivate::maxCount = 0;

StatusListPrivate::StatusListPrivate() :
        twitterapi( new TwitterAPI ),
        visible( false ),
        newStatuses(0),
        active(-1)
{}

StatusListPrivate::~StatusListPrivate()
{
    twitterapi->deleteLater();
}

void StatusListPrivate::init()
{
    twitterapi->setServiceUrl( account->serviceUrl() );
    twitterapi->setLogin( account->login() );
    twitterapi->setPassword( account->password() );
#ifdef HAVE_OAUTH
    if ( account->serviceUrl() == Account::NetworkUrlTwitter ) {
        twitterapi->setUsingOAuth( true );
        twitterapi->setConsumerKey( OAuthWizard::ConsumerKey );
        twitterapi->setConsumerSecret( OAuthWizard::ConsumerSecret );
    } else {
        twitterapi->setUsingOAuth( false );
    }
#endif

    connect( twitterapi, SIGNAL(newEntries(EntryList)), this, SLOT(addEntries(EntryList)));
    connect( twitterapi, SIGNAL(deleteEntry(quint64)), this, SLOT(deleteEntry(quint64)) );
    connect( twitterapi, SIGNAL(favoriteStatus(quint64,bool)), this, SLOT(setFavorited(quint64,bool)) );

    connect( twitterapi, SIGNAL(deleteDMDone(quint64,TwitterAPI::ErrorCode)), this, SLOT(deleteEntry(quint64)) );
    connect( twitterapi, SIGNAL(errorMessage(QString)), QTwitterApp::core(), SIGNAL(errorMessage(QString)) );
    connect( twitterapi, SIGNAL(unauthorized()), this, SLOT(slotUnauthorized()) );
    connect( twitterapi, SIGNAL(unauthorized(QString,quint64)), this, SLOT(slotUnauthorized(QString,quint64)) );
    connect( twitterapi, SIGNAL(unauthorized(quint64,Entry::Type)), this, SLOT(slotUnauthorized(quint64,Entry::Type)) );
    connect( twitterapi, SIGNAL(requestDone(int)), this, SLOT(slotRequestDone(int)) );
}

void StatusListPrivate::addEntries( const EntryList &entries )
{
    foreach( const Entry &entry, entries ) {
        addEntry(entry);
    }
    StatusModel::instance()->updateDisplay();
}

void StatusListPrivate::addEntry( const Entry &entry )
{
    int index = addStatus( entry );

    if ( index != -1 ) {
        newStatuses++;

        if ( entry.type == Entry::Status ) {
            if ( ImageDownload::instance()->contains( entry.userInfo.imageUrl ) ) {
                if ( !ImageDownload::instance()->imageFromUrl( entry.userInfo.imageUrl )->isNull() )
                    setImageForUrl( entry.userInfo.imageUrl, ImageDownload::instance()->imageFromUrl( entry.userInfo.imageUrl ) );
            } else {
                ImageDownload::instance()->imageGet( entry.userInfo.imageUrl );
            }
        }
    }
}

void StatusListPrivate::deleteEntry( quint64 id )
{
    Q_Q(StatusList);

    for ( int i = 0; i < data.size(); ++i ) {
        if ( id == data.at(i).entry.id ) {
            data.removeAt(i);
            if ( active > i )
                active--;
            else if ( i < data.size() && active == i )
                data[active].state = StatusModel::STATE_ACTIVE;
            emit q->statusDeleted(i);
            return;
        }
    }
    foreach( Status status, data ) {
        if ( id == status.entry.id ) {
            int index = data.indexOf( status );
            data.removeAt( index );
            emit q->statusDeleted( index );

            return;
        }
    }
}

void StatusListPrivate::setFavorited( quint64 id, bool favorited )
{
    Q_Q(StatusList);

    for ( int i = 0; i < data.size(); i++ ) {
        Status &status = data[i];
        if ( id == status.entry.id ) {
            int index = data.indexOf( status );
            status.entry.favorited = favorited;
            emit q->favoriteChanged( index );
        }
    }
}

void StatusListPrivate::slotUnauthorized()
{
    Q_Q(StatusList);

    bool result = QTwitterApp::core()->retryAuthorizing( account, TwitterAPI::ROLE_FRIENDS_TIMELINE );
    QTwitterApp::core()->decrementRequestCount();
    if ( account->dm() )
        QTwitterApp::core()->decrementRequestCount();
    if ( !result )
        return;
    twitterapi->setLogin( account->login() );
    twitterapi->setPassword( account->password() );
    q->requestFriendsTimeline();
    q->requestMentions();
    if ( account->dm() )
        q->requestDirectMessages();
}

void StatusListPrivate::slotUnauthorized( const QString &status, quint64 inReplyToId )
{
    Q_Q(StatusList);

    bool result = QTwitterApp::core()->retryAuthorizing( account, TwitterAPI::ROLE_POST_UPDATE );
    QTwitterApp::core()->decrementRequestCount();
    if ( !result )
        return;
    twitterapi->setLogin( account->login() );
    twitterapi->setPassword( account->password() );
    q->requestNewStatus( status, inReplyToId );
}

void StatusListPrivate::slotUnauthorized( const QString &screenName, const QString &text )
{
    Q_Q(StatusList);

    bool result = QTwitterApp::core()->retryAuthorizing( account, TwitterAPI::ROLE_POST_DM );
    QTwitterApp::core()->decrementRequestCount();
    if ( !result )
        return;
    twitterapi->setLogin( account->login() );
    twitterapi->setPassword( account->password() );
    q->requestNewDM( screenName, text );
}

void StatusListPrivate::slotUnauthorized( quint64 destroyId, Entry::Type type )
{
    Q_Q(StatusList);

    bool result = QTwitterApp::core()->retryAuthorizing( account, TwitterAPI::ROLE_DELETE_UPDATE );
    QTwitterApp::core()->decrementRequestCount();
    if ( !result )
        return;
    twitterapi->setLogin( account->login() );
    twitterapi->setPassword( account->password() );
    q->requestDestroy( destroyId, type );
}

void StatusListPrivate::slotRequestDone( int role )
{
    if ( visible )
        StatusModel::instance()->updateDisplay();
    if ( role != TwitterAPI::ROLE_POST_DM && QTwitterApp::core()->requestCount() > 0 )
        QTwitterApp::core()->decrementRequestCount();
    qDebug() << QTwitterApp::core()->requestCount();
    //  if ( Core::requestCount() == 0 ) {
    //    if ( checkForNew )
    //      core->checkUnreadStatuses();
    //    emit resetUi();
    //  }
    //  checkForNew = true;
}

void StatusListPrivate::setImageForUrl( const QString &url, QPixmap *pixmap )
{
    Q_Q(StatusList);
    for( int i = 0; i < data.size(); ++i ) {
        if ( data.at(i).entry.type == Entry::Status && url == data.at(i).entry.userInfo.imageUrl ) {
            data[i].image = *pixmap;
            emit q->imageChanged( i );
        }
    }
}

StatusList::StatusList( Account *account, QObject *parent ) :
        QObject( parent ),
        d_ptr( new StatusListPrivate )
{
    Q_D(StatusList);

    d->q_ptr = this;
    d->account = account;
    d->init();
}

StatusList::~StatusList()
{
    delete d_ptr;
}

int StatusList::newStatusesCount()
{
    Q_D(StatusList);

    int count = d->newStatuses;
    d->newStatuses = 0;

    return count;
}

void StatusList::markAllAsRead()
{
    Q_D(StatusList);

    for ( int i = 0; i < d->data.size(); ++i ) {
        d->data[i].state = StatusModel::STATE_READ;
        emit stateChanged(i);
    }
}

bool StatusList::dm() const
{
    Q_D(const StatusList);

    return d->account->dm();
}

QString StatusList::serviceUrl() const
{
    Q_D(const StatusList);

    return d->account->serviceUrl();
}

QString StatusList::login() const
{
    Q_D(const StatusList);

    return d->account->login();
}

void StatusList::setVisible( bool visible )
{
    Q_D(StatusList);

    d->visible = visible;
}

bool StatusList::isVisible() const
{
    Q_D(const StatusList);

    return d->visible;
}

void StatusList::setData( int index, const Status &status )
{
    Q_D(StatusList);

    d->data[ index ] = status;
    if ( status.state == StatusModel::STATE_ACTIVE ) {
        d->active = index;
        //    emit stateChanged( index );
    }
    emit dataChanged( index );
}

const Status& StatusList::data( int index ) const
{
    Q_D(const StatusList);

    return d->data.at( index );
}

void StatusList::setState( int index, StatusModel::StatusState state )
{
    Q_D(StatusList);

    if ( d->data[ index ].state == state )
        return;

    if ( d->data[ index ].state == StatusModel::STATE_ACTIVE )
        d->active = -1;

    d->data[ index ].state = state;

    if ( state == StatusModel::STATE_ACTIVE )
        d->active = index;

    emit stateChanged( index );
}

StatusModel::StatusState StatusList::state( int index ) const
{
    Q_D(const StatusList);

    return d->data[ index ].state;
}

void StatusList::setImage( int index, const QPixmap &pixmap )
{
    Q_D(StatusList);

    d->data[ index ].image = pixmap;
    emit imageChanged( index );
}

const QList<Status>& StatusList::getData() const
{
    Q_D(const StatusList);

    return d->data;
}

void StatusList::setStatuses( const QList<Status> &statuses )
{
    Q_D(StatusList);

    d->data = statuses;
}


int StatusList::active() const
{
    Q_D(const StatusList);

    return d->active;
}

void StatusList::setActive( int active )
{
    Q_D(StatusList);

    d->active = active;
}

int StatusList::size() const
{
    Q_D(const StatusList);

    return d->data.size();
}


void StatusList::requestFriendsTimeline()
{
    Q_D(StatusList);

    d->twitterapi->friendsTimeline( StatusListPrivate::maxCount );
    QTwitterApp::core()->incrementRequestCount();
}

void StatusList::requestMentions()
{
    Q_D(StatusList);

    d->twitterapi->mentions( StatusListPrivate::maxCount );
    QTwitterApp::core()->incrementRequestCount();
}

void StatusList::requestDirectMessages()
{
    Q_D(StatusList);

    d->twitterapi->directMessages( StatusListPrivate::maxCount );
    QTwitterApp::core()->incrementRequestCount();
}

void StatusList::requestNewStatus( const QString &status, quint64 inReplyTo )
{
    Q_D(StatusList);

    d->twitterapi->postUpdate( status, inReplyTo );
    QTwitterApp::core()->incrementRequestCount( Core::DontCheckForUnread );
}

void StatusList::requestNewDM( const QString &screenName, const QString &text )
{
    Q_D(StatusList);

    d->twitterapi->postDM( screenName, text );
    // DMDialog has it's own spinner icon, so don't increment core's request count
}

void StatusList::postDMDialog( const QString &screenName )
{
    Q_D(StatusList);
    DMDialog *dlg = new DMDialog( screenName, QTwitterApp::instance()->activeWindow() );
    connect( dlg, SIGNAL(dmRequest(QString,QString)), this, SLOT(requestNewDM(QString,QString)) );
    connect( d->twitterapi, SIGNAL(postDMDone(TwitterAPI::ErrorCode)), dlg, SLOT(showResult(TwitterAPI::ErrorCode)) );

    dlg->exec();
    dlg->deleteLater();
}

void StatusList::requestDestroy( quint64 id, Entry::Type type )
{
    Q_D(StatusList);

    if ( type == Entry::Status ) {
        d->twitterapi->deleteUpdate( id );
    } else {
        d->twitterapi->deleteDM( id );
    }
    QTwitterApp::core()->incrementRequestCount( Core::DontCheckForUnread );
}

void StatusList::requestCreateFavorite( quint64 id )
{
    Q_D(StatusList);

    d->twitterapi->createFavorite( id );
    QTwitterApp::core()->incrementRequestCount( Core::DontCheckForUnread );
}

void StatusList::requestDestroyFavorite( quint64 id )
{
    Q_D(StatusList);

    d->twitterapi->destroyFavorite( id );
    QTwitterApp::core()->incrementRequestCount( Core::DontCheckForUnread );
}


int StatusListPrivate::addStatus( const Entry &entry )
{
    for ( QList<Status>::const_iterator i = data.begin(); i != data.end(); ++i) {
        if ( entry.id == (*i).entry.id ) {
            //      qDebug() << "found existing entry of the same id";
            return -1;
        }
    }
    //  qDebug() << "adding new entry";

    Status status;
    status.state = StatusModel::STATE_UNREAD;
    status.entry = entry;
    if ( status.entry.type == Entry::DirectMessage )
        status.image = QPixmap( ":/icons/mail_48.png" );

    if ( data.isEmpty() ) {
        data.append( status );
        return data.size() - 1;
    }
    for ( QList<Status>::iterator i = data.begin(); i != data.end(); ++i ) {
        if ( status.entry.timestamp > (*i).entry.timestamp ) {
            // TODO: HACK!
            int index = data.indexOf(*i);
            if ( index > 1 && data.at( index - 1 ).state != StatusModel::STATE_UNREAD ) {
                status.state = StatusModel::STATE_READ;
            }
            data.insert( i, status );
            if ( data.size() >= maxCount && data.takeLast() == status )
                return -1;
            if ( active >= data.indexOf( status ) ) {
                active++;
            }
            return data.indexOf( status );
        }
    }
    if ( data.size() < maxCount ) {
        // TODO: HACK!
        if ( data.at( data.size() - 1 ).state != StatusModel::STATE_UNREAD ) {
            status.state = StatusModel::STATE_READ;
        }
        data.append( status );
        return data.size() - 1;
    }
    return -1;
}

bool StatusList::remove( int from, int count )
{
    Q_D(StatusList);

    if ( d->data.size() < from )
        return false;

    for ( int i = count - 1; i >= 0; --i )
        d->data.removeAt( from + i );
    return true;
}

void StatusList::slotDirectMessagesChanged( bool isEnabled )
{
    Q_D(StatusList);

    if ( isEnabled )
        return;

    for ( int i = 0; i < d->data.size(); i++ ) {
        if ( d->data.at(i).entry.type == Entry::DirectMessage ) {
            d->data.removeAt(i);
            i--;
        }
    }
}

void StatusList::setMaxCount( int maxCount )
{
    StatusListPrivate::maxCount = maxCount;
}
