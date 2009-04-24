/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
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


#include <QMenu>
#include <QScrollBar>
#include <QMessageBox>
#include <QIcon>
#include <QMovie>
#include <QPalette>
#include <QShortcut>
#include <QDesktopWidget>
#include <QSignalMapper>
#include <QTreeView>
#include "mainwindow.h"
#include "tweet.h"
#include "aboutdialog.h"
#include "twitteraccountsmodel.h"
#include "twitteraccountsdelegate.h"
#include "settings.h"

extern ConfigFile settings;


MainWindow::MainWindow( QWidget *parent ) :
    QWidget( parent ),
    resetUiWhenFinished( false )
{
  ui.setupUi( this );
  ui.accountsComboBox->setVisible( false );

  progressIcon = new QMovie( ":/icons/progress.gif", "gif", this );
  ui.countdownLabel->setMovie( progressIcon );
  ui.countdownLabel->setToolTip( tr( "%n character(s) left", "", ui.countdownLabel->text().toInt() ) );

  createConnections();
  createMenu();
  createTrayIcon();
}

MainWindow::~MainWindow() {
  settings.setValue( "TwitterAccounts/currentModel", ui.accountsComboBox->currentIndex() );
}

void MainWindow::createConnections()
{
  QShortcut *replyShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_R ), this, SLOT(tweetReplyAction()) );
  QShortcut *retweetShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ), this, SLOT(tweetRetweetAction()) );
  QShortcut *copylinkShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ), this, SLOT(tweetCopylinkAction()) );
  QShortcut *deleteShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace ), this, SLOT(tweetDeleteAction()) );
  QShortcut *markallasreadShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ), this, SLOT(tweetMarkallasreadAction()) );
  QShortcut *gototwitterpageShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_T ), this, SLOT(tweetGototwitterpageAction()) );
  QShortcut *gotohomepageShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_H ), this, SLOT(tweetGotohomepageAction()) );

  Q_UNUSED(replyShortcut);
  Q_UNUSED(retweetShortcut);
  Q_UNUSED(copylinkShortcut);
  Q_UNUSED(deleteShortcut);
  Q_UNUSED(markallasreadShortcut);
  Q_UNUSED(gototwitterpageShortcut);
  Q_UNUSED(gotohomepageShortcut);


  StatusFilter *filter = new StatusFilter( this );
  ui.statusEdit->installEventFilter( filter );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SIGNAL( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SIGNAL(settingsDialogRequested()) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( editingFinished() ), this, SLOT( resetStatus() ) );
  connect( ui.statusEdit, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( ui.accountsComboBox, SIGNAL(activated(int)), this, SLOT(configSaveCurrentModel(int)) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( filter, SIGNAL( shortenUrlPressed() ), ui.statusEdit, SLOT( shortenUrl() ));
  connect( this, SIGNAL(addReplyString(QString,int)), ui.statusEdit, SLOT(addReplyString(QString,int)) );
  connect( this, SIGNAL(addRetweetString(QString)), ui.statusEdit, SLOT(addRetweetString(QString)) );
  connect( ui.statusEdit, SIGNAL( shortenUrl( QString ) ), this, SIGNAL( shortenUrl( QString ) ) );

  QShortcut *nextAccountShortcut = new QShortcut( QKeySequence( QKeySequence::MoveToNextWord ), this ); //used separately in retranslateUi()
  QShortcut *prevAccountShortcut = new QShortcut( QKeySequence( QKeySequence::MoveToPreviousWord ), this );
  connect( nextAccountShortcut, SIGNAL(activated()), this, SLOT(selectNextAccount()) );
  connect( prevAccountShortcut, SIGNAL(activated()), this, SLOT(selectPrevAccount()) );

  QShortcut *hideShortcut = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_H ), this );
  connect( hideShortcut, SIGNAL(activated()), this, SLOT(hide()) );
#ifdef Q_WS_MAC
  ui.settingsButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Comma ) );
#else
  ui.settingsButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
#endif
  ui.updateButton->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_R ) );
}

void MainWindow::createMenu()
{
  buttonMenu = new QMenu( this );
  newtweetAction = new QAction( tr( "New tweet" ), buttonMenu );
  newtwitpicAction = new QAction( tr( "Upload a photo to TwitPic" ), buttonMenu );
  gototwitterAction = new QAction( tr( "Go to Twitter" ), buttonMenu );
  gototwitpicAction = new QAction( tr( "Go to TwitPic" ), buttonMenu );
  quitAction = new QAction( tr( "Quit" ), buttonMenu );
  quitAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );
  quitAction->setShortcutContext( Qt::ApplicationShortcut );
  connect( quitAction, SIGNAL(triggered()), qApp, SLOT(quit()) );

  newtweetAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  newtwitpicAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
  gototwitterAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  gototwitpicAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_G ) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( gototwitterAction, "http://twitter.com/home" );
  mapper->setMapping( gototwitpicAction, "http://twitpic.com" );

  connect( newtweetAction, SIGNAL(triggered()), ui.statusEdit, SLOT(setFocus()) );
  connect( newtwitpicAction, SIGNAL(triggered()), this, SIGNAL(openTwitPicDialog()) );
  connect( gototwitterAction, SIGNAL(triggered()), mapper, SLOT(map()) );
  connect( gototwitpicAction, SIGNAL(triggered()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(QString)), this, SLOT(emitOpenBrowser(QString)) );

  buttonMenu->addAction( newtweetAction );
  buttonMenu->addAction( newtwitpicAction );
  buttonMenu->addSeparator();
  buttonMenu->addAction( gototwitterAction );
  buttonMenu->addAction( gototwitpicAction );
  buttonMenu->addSeparator();
  buttonMenu->addAction( quitAction );
  ui.moreButton->setMenu( buttonMenu );
}

void MainWindow::createTrayIcon()
{
  trayIcon = new QSystemTrayIcon( this );
  trayIcon->setIcon( QIcon( ":/icons/twitter_48.png" ) );

  connect( trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)) );
  connect( trayIcon, SIGNAL(messageClicked()), this, SLOT(show()) );
#ifndef Q_WS_MAC
  QMenu *trayMenu = new QMenu( this );
  trayMenu = new QMenu( this );
  QAction *quitaction = new QAction( tr( "Quit" ), trayMenu);
  QAction *settingsaction = new QAction( tr( "Settings" ), trayMenu);
  settingsaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_S ) );
  quitaction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );

  connect( quitaction, SIGNAL(triggered()), qApp, SLOT(quit()) );
  connect( settingsaction, SIGNAL(triggered()), this, SIGNAL(settingsDialogRequested()) );
  connect( settingsaction, SIGNAL(triggered()), this, SLOT(show()) );

  trayMenu->addAction(settingsaction);
  trayMenu->addSeparator();
  trayMenu->addAction(quitaction);
  trayIcon->setContextMenu( trayMenu );

  trayIcon->setToolTip( "qTwitter" );
#endif
  trayIcon->show();
}

StatusList* MainWindow::getListView()
{
  return ui.statusListView;
}

int MainWindow::getScrollBarWidth()
{
  return ui.statusListView->verticalScrollBar()->size().width();
}

void MainWindow::setupTwitterAccounts( const QList<TwitterAccount> &accounts, bool publicTimeline )
{
  ui.accountsComboBox->clear();

  foreach ( TwitterAccount account, accounts ) {
    if ( account.isEnabled )
      ui.accountsComboBox->addItem( account.login );
  }

  if ( ( !publicTimeline && accounts.size() < 2 ) || accounts.isEmpty() ) {
    ui.accountsComboBox->setVisible( false );
    if ( !accounts.isEmpty() )
      emit switchModel( accounts.at(0).login );
    else
      emit switchToPublicTimelineModel();
    ui.statusEdit->setEnabled( !( ui.accountsComboBox->currentText() == tr( "public timeline" ) ) );
    return;
  }

  if ( publicTimeline )
    ui.accountsComboBox->addItem( tr( "public timeline" ) );

  if ( ui.accountsComboBox->count() <= 1 ) {
    ui.accountsComboBox->setVisible( false );

    if ( ui.accountsComboBox->currentText() == tr( "public timeline" ) )
      emit switchToPublicTimelineModel();
    else
      emit switchModel( ui.accountsComboBox->currentText() );
    ui.statusEdit->setEnabled( !( ui.accountsComboBox->currentText() == tr( "public timeline" ) ) );
    return;
  }
  ui.accountsComboBox->setVisible( true );

  int index = settings.value( "TwitterAccounts/currentModel", 0 ).toInt();

  if ( index < 0 || index >= ui.accountsComboBox->count() )
    ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->count() - 1 );
  else
    ui.accountsComboBox->setCurrentIndex( index );

  if ( ui.accountsComboBox->currentText() == tr( "public timeline" ) )
    emit switchToPublicTimelineModel();
  else
    emit switchModel( ui.accountsComboBox->currentText() );
  ui.statusEdit->setEnabled( !( ui.accountsComboBox->currentText() == tr( "public timeline" ) ) );
}

void MainWindow::setListViewModel( TweetModel *model )
{
  if ( !model )
    return;
  TweetModel *currentModel = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( currentModel ) {
    if ( model == currentModel )
      return;
    currentModel->setVisible( false );
  }
  ui.statusListView->setModel( model );
  model->display();
}

void MainWindow::closeEvent( QCloseEvent *e )
{
  if ( trayIcon->isVisible()) {
    hide();
    e->ignore();
    return;
  }
  QWidget::closeEvent( e );
}

void MainWindow::iconActivated( QSystemTrayIcon::ActivationReason reason )
{
  switch ( reason ) {
    case QSystemTrayIcon::Trigger:
#ifdef Q_WS_WIN
    if ( !isVisible() ) {
#else
    if ( !isVisible() || !QApplication::activeWindow() ) {
#endif
      show();
        raise();
        activateWindow();
      } else {
        hide();
      }
      break;
    default:
      break;
  }
}

void MainWindow::changeLabel()
{
  QString toolTip = tr( "%n character(s) left", "", ui.statusEdit->charsLeft() );
  QPalette palette( ui.countdownLabel->palette() );

  if( !ui.statusEdit->isStatusClean() ) {
    if ( ui.statusEdit->charsLeft() < 0 ) {
      palette.setColor( QPalette::Foreground, Qt::red );
      toolTip = tr( "%n character(s) over the limit", "", ui.statusEdit->charsLeft() * -1 );
    } else {
      palette.setColor( QPalette::Foreground, Qt::black );
    }
  }

  ui.countdownLabel->setText( QString::number( ui.statusEdit->charsLeft() ) );
  ui.countdownLabel->setPalette( palette );
  ui.countdownLabel->setToolTip( toolTip );
}

void MainWindow::sendStatus()
{
  if( ui.statusEdit->charsLeft() < 0 ) {
    QMessageBox *messageBox = new QMessageBox( QMessageBox::Warning, tr( "Message too long" ), tr( "Your message is too long." ) );
    QPushButton *accept = messageBox->addButton( tr( "Cool" ), QMessageBox::AcceptRole );
    QPushButton *reject = messageBox->addButton( tr( "Oops..." ), QMessageBox::RejectRole );
    messageBox->setInformativeText( tr( "You can still post it like this, but it will be truncated." ) );
    messageBox->setDefaultButton( accept );
    messageBox->setEscapeButton( reject );
    messageBox->exec();
    if ( messageBox->clickedButton() == reject )
      return;
    messageBox->deleteLater();
  }
  resetUiWhenFinished = true;
  emit post( ui.accountsComboBox->currentText(), ui.statusEdit->text(), ui.statusEdit->getInReplyTo() );
  showProgressIcon();
}

void MainWindow::resetStatusEdit()
{
  if ( resetUiWhenFinished ) {
    resetUiWhenFinished = false;
    ui.statusEdit->cancelEditing();
  }
  progressIcon->stop();
  changeLabel();
}

void MainWindow::showProgressIcon()
{
  ui.countdownLabel->clear();
  ui.countdownLabel->setMovie( progressIcon );
  progressIcon->start();
}

void MainWindow::configSaveCurrentModel( int index )
{
  if ( settings.value( "TwitterAccounts/currentModel", 0 ).toInt() != index ) {
    settings.setValue( "TwitterAccounts/currentModel", index );
    if ( ui.accountsComboBox->currentText() == tr( "public timeline" ) )
      emit switchModel( "public timeline" );
    else
      emit switchModel( ui.accountsComboBox->currentText() );
  }
  ui.statusEdit->setEnabled( !( ui.accountsComboBox->currentText() == tr( "public timeline" ) ) );
}

void MainWindow::selectNextAccount()
{
  if ( ui.accountsComboBox->currentIndex() < ui.accountsComboBox->count() - 1 ) {
    ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->currentIndex() + 1 );
    configSaveCurrentModel( ui.accountsComboBox->currentIndex() );
  }
}

void MainWindow::selectPrevAccount()
{
  if ( ui.accountsComboBox->currentIndex() > 0 ) {
    ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->currentIndex() - 1 );
    configSaveCurrentModel( ui.accountsComboBox->currentIndex() );
  }
}

void MainWindow::resetStatus()
{
  if ( ui.statusEdit->isStatusClean() ) {
    changeLabel();
  }
}

void MainWindow::resizeEvent( QResizeEvent *event )
{
  emit resizeView( event->size().width(), event->oldSize().width() );
}

void MainWindow::popupMessage( QString message )
{
  if( settings.value( "General/notifications" ).toBool() ) {
    //: The full sentence is e.g.: "New tweets for <user A>, <user B> and the public timeline"
    message.replace( "public timeline", tr( "the public timeline" ) );
    trayIcon->showMessage( tr( "New tweets" ), message, QSystemTrayIcon::Information );
  }
}

void MainWindow::popupError( const QString &message )
{
  QMessageBox::information( this, tr("Error"), message );
}

void MainWindow::emitOpenBrowser( QString address )
{
  emit openBrowser( QUrl( address ) );
}

void MainWindow::changeListBackgroundColor(const QColor &newColor )
{
  QPalette palette( ui.statusListView->palette() );
  palette.setColor( QPalette::Base, newColor );
  ui.statusListView->setPalette( palette );
  ui.statusListView->update();
}

void MainWindow::about()
{
  AboutDialog dlg;
  dlg.exec();
}

void MainWindow::retranslateUi()
{
  ui.updateButton->setToolTip( QString("%1 <span style=\"color: gray\">%2</span>").arg( tr( "Update tweets" ) ).arg( ui.updateButton->shortcut().toString( QKeySequence::NativeText ) ) );
  ui.settingsButton->setToolTip( QString("%1 <span style=\"color: gray\">%2</span>").arg( tr( "Settings" ), ui.settingsButton->shortcut().toString( QKeySequence::NativeText ) ) );
  ui.accountsComboBox->setToolTip( tr( "Navigate using %1 and %2" )
                                   .arg( QString( "<span style=\"color: gray\">%1</span>" )
                                         .arg(QKeySequence( QKeySequence::MoveToPreviousWord ).toString( QKeySequence::NativeText ) ) )
                                   .arg( QString( "<span style=\"color: gray\">%1</span>" )
                                         .arg(QKeySequence( QKeySequence::MoveToNextWord ).toString( QKeySequence::NativeText ) ) ) );
  ui.moreButton->setToolTip( tr("More...") );
  if ( ui.statusEdit->isStatusClean() ) {
    ui.statusEdit->initialize();
  }
  ui.statusEdit->setText( tr( "What are you doing?" ) );
  newtweetAction->setText( tr( "New tweet" ) );
  newtwitpicAction->setText( tr( "Upload a photo to TwitPic" ) );
  gototwitterAction->setText( tr( "Go to Twitter" ) );
  gototwitpicAction->setText( tr( "Go to TwitPic" ) );
  quitAction->setText( tr( "Quit" ) );
}

void MainWindow::replaceUrl( const QString &url )
{
    QString text = ui.statusEdit->text();
    text.replace( ui.statusEdit->getSelectedUrl(), url );
    ui.statusEdit->setText( text );
    ui.statusEdit->setCursorPosition( text.indexOf( url ) + url.length() );
}

void MainWindow::tweetReplyAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      model->currentTweet()->slotReply();
    }
}

void MainWindow::tweetRetweetAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      model->currentTweet()->slotRetweet();
    }
}

void MainWindow::tweetCopylinkAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      model->currentTweet()->slotCopyLink();
    }
}

void MainWindow::tweetDeleteAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      model->currentTweet()->slotDelete();
    }
}

void MainWindow::tweetMarkallasreadAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    model->markAllAsRead();
}

void MainWindow::tweetGototwitterpageAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      emitOpenBrowser( "http://twitter.com/" + model->currentTweet()->data().login );
    }
}

void MainWindow::tweetGotohomepageAction()
{
  TweetModel *model = qobject_cast<TweetModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentTweet() )
    {
      emitOpenBrowser( model->currentTweet()->data().homepage );
    }
}

/*! \class MainWindow
    \brief A class defining the main window of the application.

    This class contains all the GUI elements of the main application window.
    It receives signals from Core and TweetModel classes and provides means
    of visualization for them.
*/

/*! \fn MainWindow::MainWindow( QWidget *parent = 0 )
    A default constructor. Creates a MainWindow instance with the given \a parent.
*/

/*! \fn MainWindow::~MainWindow()
    A default destructor.
*/

/*! \fn StatusList* MainWindow::getListView()
    A method for external access to the list view used for displaying Tweets.
    Used for initialization of TweetModel class's instance.
    \returns A pointer to the list view instance of MainWindow.
*/

/*! \fn int MainWindow::getScrollBarWidth()
    A method for accessing the list view scrollbar's width, needed for computing width
    of Tweet class instances.
    \returns List view scrollbar's width.
*/

/*! \fn void MainWindow::setListViewModel( TweetModel *model )
    Assigns the \a model to be a list view model.
    \param model The model for the list view.
*/

/*! \fn void MainWindow::changeListBackgroundColor( const QColor &newColor )
    Sets the background color of a list view. Used when changing color theme.
    \param newColor List view new background color.
*/

/*! \fn void MainWindow::popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages )
    Pops up a tray icon notification message containing information about
    new Tweets and direct messages (if any). Displays total messages count and
    their authors' names for status updates and direct messages separately.
    \param statusesCount The amount of new status updates.
    \param namesForStatuses List of statuses senders' names.
    \param messagesCount The amount of new direct messages.
    \param namesForMessages List of direct messages senders' names.
*/

/*! \fn void MainWindow::popupError( const QString &message )
    Pops up a dialog with an error or information for User. This is an interface
    for all the classes that notify User about any problems (e.g. Core, TwitterAPI
    and ImageDownload).
    \param message An information to be shown to User.
*/

/*! \fn void MainWindow::retranslateUi()
    Retranslates all the translatable GUI elements of the class. Used when changing
    UI language on the fly.
*/

/*! \fn void MainWindow::resetStatusEdit()
    Resets status edit field if necessary. Invoked mainly when updating timeline finishes.
*/

/*! \fn void MainWindow::showProgressIcon()
    Displays progress icon when processing a request.
*/

/*! \fn void MainWindow::about()
    Pops up a small dialog with credits and short info on the application and its author.
*/

/*! \fn void MainWindow::updateTweets()
    Emitted to force timeline update, assigned to pressing the update button.
*/

/*! \fn void MainWindow::openTwitPicDialog()
    Emitted to open a dialog for uploading a photo to TwitPic.
*/

/*! \fn void MainWindow::post( const QByteArray& status, int inReplyTo = -1 )
    Emitted to post a status update. Assigned to pressing Enter inside the status edit field.
    \param status A status to be posted.
    \param inReplyTo In case the status is a reply - optional id of the existing status to which the reply is posted.
*/

/*! \fn void MainWindow::openBrowser( QUrl address )
    Emitted when "Go to..." action requested, asks to open a default browser.
    \param address Requested URL.
    \sa Core::openBrowser()
*/

/*! \fn void MainWindow::settingsDialogRequested()
    Emitted when settings button pressed, requests opening the settings dialog.
*/

/*! \fn void MainWindow::addReplyString( const QString& user, int inReplyTo )
    Works as a proxy between Tweet class instance and status edit field. Passes the request
    to initiate editing a reply.
    \param user Login of a User to whom the current User replies.
    \param inReplyTo Id of the existing status to which the reply is posted.
*/

/*! \fn void MainWindow::addRetweetString( QString message )
    Works as a proxy between Tweet class instance and status edit field. Passes the request
    to initiate editing a retweet.
    \param message A retweet message
*/

/*! \fn void MainWindow::resizeView( int width, int oldWidth )
    Emitted when resizing a window, to inform all the Tweets about the size change.
    \param width The width after resizing.
    \param oldWidth The width before resizing.
*/

/*! \fn void MainWindow::closeEvent( QCloseEvent *e )
    An event reimplemented in order to provide hiding instead of closing the application.
    Closing is provided only via a shortcut or tray icon menu option.
    \param e A QCloseEvent event's representation.
*/
