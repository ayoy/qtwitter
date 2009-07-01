/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
 *   Copyright (C) 2009 by Mariusz Pietrzyk       <wijet@wijet.pl>         *
 *   Copyright (C) 2009 by Anna Nowak           <wiorka@gmail.com>         *
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


#include <QMenu>
#include <QMenuBar>
#include <QScrollBar>
#include <QMessageBox>
#include <QIcon>
#include <QMovie>
#include <QPalette>
#include <QShortcut>
#include <QDesktopWidget>
#include <QSignalMapper>
#include <QTreeView>
#include <QTimer>
#include <qticonloader.h>
#include <twitterapi/twitterapi.h>
#include "mainwindow.h"
#include "statuswidget.h"
#include "aboutdialog.h"
#include "account.h"
#include "accountsdelegate.h"
#include "accountscontroller.h"
#include "configfile.h"
#include "updater.h"

extern ConfigFile settings;


MainWindow::MainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    resetUiWhenFinished( false )
{
  QWidget *centralWidget = new QWidget( this );
  ui.setupUi( centralWidget );
  setCentralWidget( centralWidget );

  StatusWidget::setScrollBarWidth( ui.statusListView->verticalScrollBar()->width() );

//  ui.accountsComboBox->setVisible( false );

  timer = new QTimer( this );
  progressIcon = new QMovie( ":/icons/progress.gif", "gif", this );
  ui.countdownLabel->setMovie( progressIcon );
  ui.countdownLabel->setToolTip( tr( "%n characters left", "", StatusEdit::STATUS_MAX_LENGTH ) );
  ui.statusEdit->setToolTip( ui.statusEdit->toolTip().arg( QKeySequence( Qt::CTRL + Qt::Key_J ).toString( QKeySequence::NativeText ) ) );

  //> experiment begin
  ui.moreButton->setIcon( QtIconLoader::icon("list-add", QIcon(":/icons/add_48.png")) );
  ui.settingsButton->setIcon( QtIconLoader::icon("preferences-other", QIcon(":/icons/spanner_48.png")) );
  ui.updateButton->setIcon( QtIconLoader::icon("reload", QIcon(":icons/refresh_48.png")) );
  //< experiment end

  createConnections();
  createButtonMenu();
  createTrayIcon();

// create menu bar only on maemo
#ifdef Q_WS_HILDON
  createHildonMenu();
#endif
  if ( settings.value( "Network/updates/check", true ).toBool() ) {
    silentCheckForUpdates();
  }
}

MainWindow::~MainWindow() {
  settings.setValue( "Accounts/visibleAccount", ui.accountsComboBox->currentIndex() );
}

void MainWindow::createConnections()
{
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_R ), this, SLOT(statusReplyAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_T ), this, SLOT(statusRetweetAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_C ), this, SLOT(statusCopylinkAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Backspace ), this, SLOT(statusDeleteAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_A ), this, SLOT(statusMarkallasreadAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_A ), this, SIGNAL(statusMarkeverythingasreadAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_T ), this, SLOT(statusGototwitterpageAction()) );
  new QShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_H ), this, SLOT(statusGotohomepageAction()) );

  StatusFilter *filter = new StatusFilter( this );
  ui.statusEdit->installEventFilter( filter );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SIGNAL( updateStatuses() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), this, SIGNAL(settingsDialogRequested()) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( editingFinished() ), this, SLOT( resetStatus() ) );
  connect( ui.statusEdit, SIGNAL(errorMessage(QString)), this, SLOT(popupError(QString)) );
  connect( ui.accountsComboBox, SIGNAL(activated(int)), this, SLOT(configSaveCurrentModel(int)) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( filter, SIGNAL( shortenUrlPressed() ), ui.statusEdit, SLOT( shortenUrl() ));
  connect( this, SIGNAL(addReplyString(QString,quint64)), ui.statusEdit, SLOT(addReplyString(QString,quint64)) );
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

#ifdef Q_WS_HILDON
void MainWindow::createHildonMenu()
{
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction( aboutAction );
  fileMenu->addAction( quitAction );
}
#endif

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

void MainWindow::createButtonMenu()
{
  buttonMenu = new QMenu( this );
  newstatusAction = new QAction( tr( "New tweet" ), buttonMenu );
  newtwitpicAction = new QAction( tr( "Upload a photo to TwitPic" ), buttonMenu );
  gototwitterAction = new QAction( tr( "Go to Twitter" ), buttonMenu );
  gotoidenticaAction = new QAction( tr( "Go to Identi.ca" ), buttonMenu );
  gototwitpicAction = new QAction( tr( "Go to TwitPic" ), buttonMenu );
  checkforupdatesAction = new QAction( tr( "Check for updates" ), buttonMenu );
  aboutAction = new QAction( tr( "About qTwitter..." ), buttonMenu );
  quitAction = new QAction( tr( "Quit" ), buttonMenu );
  quitAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Q ) );
  quitAction->setShortcutContext( Qt::ApplicationShortcut );
  connect( quitAction, SIGNAL(triggered()), qApp, SLOT(quit()) );

  newstatusAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  newtwitpicAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
  gototwitterAction->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  gotoidenticaAction->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_G ) );
  gototwitpicAction->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_G ) );

  QSignalMapper *mapper = new QSignalMapper( this );
  mapper->setMapping( gototwitterAction, "http://twitter.com/home" );
  mapper->setMapping( gototwitterAction, "http://identi.ca" );
  mapper->setMapping( gototwitpicAction, "http://twitpic.com" );

  connect( newstatusAction, SIGNAL(triggered()), ui.statusEdit, SLOT(setFocus()) );
  connect( newtwitpicAction, SIGNAL(triggered()), this, SIGNAL(twitPicRequested()) );
  connect( gototwitterAction, SIGNAL(triggered()), mapper, SLOT(map()) );
  connect( gototwitpicAction, SIGNAL(triggered()), mapper, SLOT(map()) );
  connect( mapper, SIGNAL(mapped(QString)), this, SLOT(emitOpenBrowser(QString)) );
  connect( checkforupdatesAction, SIGNAL(triggered()), this, SLOT(checkForUpdates()) );
  connect( aboutAction, SIGNAL(triggered()), this, SLOT(about()) );

  buttonMenu->addAction( newstatusAction );
  buttonMenu->addAction( newtwitpicAction );
  buttonMenu->addSeparator();
  buttonMenu->addAction( gototwitterAction );
  buttonMenu->addAction( gotoidenticaAction );
  buttonMenu->addAction( gototwitpicAction );
  buttonMenu->addSeparator();
  buttonMenu->addAction( checkforupdatesAction );
  buttonMenu->addAction( aboutAction );
  buttonMenu->addAction( quitAction );
  ui.moreButton->setMenu( buttonMenu );
}

StatusListView* MainWindow::getListView()
{
  return ui.statusListView;
}

int MainWindow::getScrollBarWidth()
{
  return ui.statusListView->verticalScrollBar()->size().width();
}

void MainWindow::setupAccounts( const QList<Account> &accounts )
{
  ui.accountsComboBox->clear();
  ui.statusEdit->setEnabled( false );
  foreach ( Account account, accounts ) {
    if ( account.isEnabled )
      ui.accountsComboBox->addItem( QString( "%1 @%2" ).arg( account.login, Account::networkToString( account.network ) ) );
  }

  if ( ui.accountsComboBox->count() == 0 ) {
    ui.accountsComboBox->setVisible( false );
    ui.statusEdit->setEnabled( false );
  } else {
    int index = settings.value( "Accounts/visibleAccount", 0 ).toInt();

    if ( index < 0 || index >= ui.accountsComboBox->count() ) {
      ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->count() - 1 );
    } else {
      ui.accountsComboBox->setCurrentIndex( index );
    }

    ui.accountsComboBox->setVisible( true );
    ui.statusEdit->setEnabled( true );
    emit switchModel( Account::fromString( ui.accountsComboBox->currentText() ).first,
                      Account::fromString( ui.accountsComboBox->currentText() ).second );
  }
}

void MainWindow::setListViewModel( StatusModel *model )
{
  if ( !model )
    return;
  ui.statusListView->setModel( model );
}

void MainWindow::changeLabel()
{
  QPalette palette( ui.countdownLabel->palette() );

  int chars = ui.statusEdit->charsLeft();

  QString toolTip = (chars == 1) ? tr(  "%n character left", "", chars ) :
                                   tr( "%n characters left", "", chars );

  if( !ui.statusEdit->isStatusClean() ) {
    if ( ui.statusEdit->charsLeft() < 0 ) {
      palette.setColor( QPalette::Foreground, Qt::red );
      toolTip = (chars == -1) ? tr( "%n character over the limit", "", chars * -1 ) :
                                tr( "%n characters over the limit", "", chars * -1 );
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
  emit post( Account::fromString( ui.accountsComboBox->currentText() ).first, Account::fromString( ui.accountsComboBox->currentText() ).second, ui.statusEdit->text(), ui.statusEdit->getInReplyTo() );
  showProgressIcon();
}

void MainWindow::resetStatusEdit()
{
  if ( resetUiWhenFinished ) {
    resetUiWhenFinished = false;
    ui.statusEdit->cancelEditing();
  }
  progressIcon->stop();
  emit iconStopped();
  changeLabel();
}

void MainWindow::pauseIcon()
{
  progressIcon->stop();
  changeLabel();
}

void MainWindow::showProgressIcon()
{
  ui.countdownLabel->clear();
  ui.countdownLabel->setMovie( progressIcon );
  progressIcon->start();
  if ( !timer->isActive() )
    timer->singleShot( 60000, this, SLOT(resetStatusEdit()) );
}
void MainWindow::show()
{
  ui.statusListView->setUpdatesEnabled( false );
  QMainWindow::show();
  configSaveCurrentModel( ui.accountsComboBox->currentIndex(), true );
  ui.statusListView->setUpdatesEnabled( true );
}

void MainWindow::configSaveCurrentModel( int index, bool unconditionally )
{
  if ( settings.value( "Accounts/visibleAccount", 0 ).toInt() != index
       || unconditionally ) {
    settings.setValue( "Accounts/visibleAccount", index );
    QRegExp rx( "(.+) @(.+)" );
    if ( rx.indexIn( ui.accountsComboBox->currentText() ) == -1 )
      return;
    emit switchModel( Account::fromString( ui.accountsComboBox->currentText() ).first,
                      Account::fromString( ui.accountsComboBox->currentText() ).second );
  }
//  ui.statusEdit->setEnabled( !( ui.accountsComboBox->currentText() == tr( "public timeline" ) ) );
}

void MainWindow::selectNextAccount()
{
  ui.accountsComboBox->setCurrentIndex( (ui.accountsComboBox->currentIndex() + 1) % ui.accountsComboBox->count() );
  configSaveCurrentModel( ui.accountsComboBox->currentIndex() );
}

void MainWindow::selectPrevAccount()
{
  if ( ui.accountsComboBox->currentIndex() > 0 ) {
    ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->currentIndex() - 1 );
  } else {
    ui.accountsComboBox->setCurrentIndex( ui.accountsComboBox->count() - 1 );
  }
  configSaveCurrentModel( ui.accountsComboBox->currentIndex() );
}

void MainWindow::resetStatus()
{
  if ( ui.statusEdit->isStatusClean() ) {
    changeLabel();
  }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Escape) {
    if ( ui.statusListView->selectionModel()->hasSelection() ) {
      ui.statusListView->clearSelection();
    } else if(isVisible()) {
      hide();
    }
    event->accept();
  }
#ifdef Q_WS_HILDON
  else if (event->key() == Qt::Key_F6) {
    if (isFullScreen())
      showNormal();
    else
      showFullScreen();
//    May be needed to update display after hiding (unless showNormal() and showFullScreen() invoke show())
//    configSaveCurrentModel( ui.accountsComboBox->currentIndex(), true );
    event->accept();
  }
#endif
  else
    QWidget::keyPressEvent( event );
}

void MainWindow::closeEvent( QCloseEvent *event )
{
  if ( trayIcon->isVisible()) {
    hide();
    event->ignore();
    return;
  }
  QWidget::closeEvent( event );
}

void MainWindow::resizeEvent( QResizeEvent *event )
{
  StatusWidget::setCurrentWidth( width() );
  emit resizeView( event->size().width(), event->oldSize().width() );
}

void MainWindow::popupMessage( QString message )
{
  if( settings.value( "General/notifications" ).toBool() ) {
    //: New tweets received (this pops up in tray)
    trayIcon->showMessage( tr( "New tweets" ), message, QSystemTrayIcon::Information );
  }
}

void MainWindow::popupError( const QString &message )
{
  QMessageBox::information( this, tr("Error"), message );
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

void MainWindow::emitOpenBrowser( QString address )
{
  emit openBrowser( QUrl( address ) );
}

void MainWindow::checkForUpdates()
{
  Updater *updater = new Updater( this );
  connect( updater, SIGNAL(updateChecked(bool,QString)), this, SLOT(readUpdateReply(bool,QString)) );
  updater->checkForUpdate();
}

void MainWindow::silentCheckForUpdates()
{
  Updater *updater = new Updater( this );
  connect( updater, SIGNAL(updateChecked(bool,QString)), this, SLOT(silentReadUpdateReply(bool,QString)) );
  updater->checkForUpdate();
}

void MainWindow::readUpdateReply( bool available, const QString &version )
{
  settings.setValue( "Network/updates/last", QDateTime::currentDateTime().toString( Qt::SystemLocaleShortDate ) );
  QMessageBox *messageBox;
  if ( available ) {
    messageBox = new QMessageBox( QMessageBox::Information, tr( "Update available" ),
                     tr( "An update to qTwitter is available!" ),
                     QMessageBox::Close, this );
    messageBox->setInformativeText( tr( "Current version is %1.<br>Download it from %2" )
                                    .arg( version, "<a href='http://www.qt-apps.org/content/show.php/qTwitter?content=99087'>"
                                                   "Qt-Apps.org</a>" ) );
  } else {
    messageBox = new QMessageBox( QMessageBox::Information, tr( "No updates available" ),
                     tr( "Sorry, no updates for qTwitter are currently available" ),
                     QMessageBox::Close, this );
  }
  messageBox->setButtonText( QMessageBox::Close, tr( "Close" ) );
  messageBox->exec();
  messageBox->deleteLater();
  sender()->deleteLater();
}

void MainWindow::silentReadUpdateReply( bool available, const QString &version )
{
  settings.setValue( "Network/updates/last", QDateTime::currentDateTime().toString( Qt::SystemLocaleShortDate ) );
  if ( available ) {
    QMessageBox *messageBox;
    messageBox = new QMessageBox( QMessageBox::Information, tr( "Update available" ),
                     tr( "An update to qTwitter is available!" ),
                     QMessageBox::Close, this );
    messageBox->setInformativeText( tr( "Current version is %1.<br>Download it from %2" )
                                    .arg( version, "<a href='http://www.qt-apps.org/content/show.php/qTwitter?content=99087'>"
                                                   "Qt-Apps.org</a>" ) );
    messageBox->setButtonText( QMessageBox::Close, tr( "Close" ) );
    messageBox->exec();
    messageBox->deleteLater();
  }
  sender()->deleteLater();
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
  AboutDialog *dlg = new AboutDialog( this );
  dlg->exec();
  dlg->deleteLater();
}

void MainWindow::retranslateUi()
{
  ui.updateButton->setToolTip( QString("%1 <span style=\"color: gray\">%2</span>").arg( tr( "Update tweets" ) ).arg( ui.updateButton->shortcut().toString( QKeySequence::NativeText ) ) );
  ui.settingsButton->setToolTip( QString("%1 <span style=\"color: gray\">%2</span>").arg( tr( "Settings" ), ui.settingsButton->shortcut().toString( QKeySequence::NativeText ) ) );
  ui.retranslateUi( this );
  ui.statusEdit->setToolTip( ui.statusEdit->toolTip().arg( QKeySequence( Qt::CTRL + Qt::Key_J ).toString( QKeySequence::NativeText ) ) );
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
  newstatusAction->setText( tr( "New tweet" ) );
  newtwitpicAction->setText( tr( "Upload a photo to TwitPic" ) );
  gototwitterAction->setText( tr( "Go to Twitter" ) );
  gotoidenticaAction->setText( tr( "Go to Identi.ca" ) );
  gototwitpicAction->setText( tr( "Go to TwitPic" ) );
  checkforupdatesAction->setText( tr( "Check for updates" ) );
  aboutAction->setText( tr( "About qTwitter..." ) );
  quitAction->setText( tr( "Quit" ) );
}

void MainWindow::replaceUrl( const QString &url )
{
    QString text = ui.statusEdit->text();
    text.replace( ui.statusEdit->getSelectedUrl(), url.trimmed() );
    ui.statusEdit->setText( text );
    ui.statusEdit->setCursorPosition( text.indexOf( url ) + url.length() );
}

void MainWindow::statusReplyAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      model->currentStatus()->slotReply();
    }
}

void MainWindow::statusRetweetAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      model->currentStatus()->slotRetweet();
    }
}

void MainWindow::statusCopylinkAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      model->currentStatus()->slotCopyLink();
    }
}

void MainWindow::statusDeleteAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      model->currentStatus()->slotDelete();
    }
}

void MainWindow::statusMarkallasreadAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    model->markAllAsRead();
}

void MainWindow::statusGototwitterpageAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      emitOpenBrowser( "http://twitter.com/" + model->currentStatus()->data().userInfo.screenName );
    }
}

void MainWindow::statusGotohomepageAction()
{
  StatusModel *model = qobject_cast<StatusModel*>( ui.statusListView->model() );
  if ( model )
    if ( model->currentStatus() )
    {
      emitOpenBrowser( model->currentStatus()->data().userInfo.homepage );
    }
}

/*! \class MainWindow
    \brief A class defining the main window of the application.

    This class contains all the GUI elements of the main application window.
    It receives signals from Core and StatusModel classes and provides means
    of visualization for them.
*/

/*! \fn MainWindow::MainWindow( QWidget *parent = 0 )
    A default constructor. Creates a MainWindow instance with the given \a parent.
*/

/*! \fn MainWindow::~MainWindow()
    A default destructor.
*/

/*! \fn StatusListView* MainWindow::getListView()
    A method for external access to the list view used for displaying Statuses.
    Used for initialization of StatusModel class's instance.
    \returns A pointer to the list view instance of MainWindow.
*/

/*! \fn int MainWindow::getScrollBarWidth()
    A method for accessing the list view scrollbar's width, needed for computing width
    of StatusWidget class instances.
    \returns List view scrollbar's width.
*/

/*! \fn void MainWindow::setListViewModel( StatusModel *model )
    Assigns the \a model to be a list view model.
    \param model The model for the list view.
*/

/*! \fn void MainWindow::changeListBackgroundColor( const QColor &newColor )
    Sets the background color of a list view. Used when changing color theme.
    \param newColor List view new background color.
*/

/*! \fn void MainWindow::popupMessage( int statusesCount, QStringList namesForStatuses, int messagesCount, QStringList namesForMessages )
    Pops up a tray icon notification message containing information about
    new Statuses and direct messages (if any). Displays total messages count and
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

/*! \fn void MainWindow::updateStatuses()
    Emitted to force timeline update, assigned to pressing the update button.
*/

/*! \fn void MainWindow::openTwitPicDialog()
    Emitted to open a dialog for uploading a photo to TwitPic.
*/

/*! \fn void MainWindow::post( const QByteArray& status, quint64 inReplyTo = -1 )
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

/*! \fn void MainWindow::addReplyString( const QString& user, quint64 inReplyTo )
    Works as a proxy between StatusWidget class instance and status edit field. Passes the request
    to initiate editing a reply.
    \param user Login of a User to whom the current User replies.
    \param inReplyTo Id of the existing status to which the reply is posted.
*/

/*! \fn void MainWindow::addRetweetString( QString message )
    Works as a proxy between StatusWidget class instance and status edit field. Passes the request
    to initiate editing a retweet.
    \param message A retweet message
*/

/*! \fn void MainWindow::resizeView( int width, int oldWidth )
    Emitted when resizing a window, to inform all the Statuses about the size change.
    \param width The width after resizing.
    \param oldWidth The width before resizing.
*/

/*! \fn void MainWindow::closeEvent( QCloseEvent *e )
    An event reimplemented in order to provide hiding instead of closing the application.
    Closing is provided only via a shortcut or tray icon menu option.
    \param e A QCloseEvent event's representation.
*/
