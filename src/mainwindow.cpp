#include "mainwindow.h"
#include "tweet.h"

#include <QMenu>
#include <QScrollBar>
#include <QMessageBox>

MainWindow::MainWindow() : QWidget(), model( 0, 0, this )
{
  ui.setupUi( this );
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + tr( " characters left" ) );
  filter = new StatusFilter();
  fm = new QFontMetrics( ui.statusListView->font() );
  settingsDialog = new Settings( this );
  loadConfig();
  ui.statusEdit->installEventFilter( filter );
  ui.statusListView->setModel( &model );
//  proxy.setType( QNetworkProxy::NoProxy );

  menu = new QMenu( this );
  QAction *openaction = new QAction("Open", this);
  QAction *closeaction = new QAction("Close", this);
  QAction *deleteaction = new QAction("Delete", this);
  QAction *aboutaction = new QAction("About", this);

  menu->addAction(openaction);
  menu->addAction(closeaction);
  menu->addAction(deleteaction);
  menu->addAction(aboutaction);

  qDebug() << qRegisterMetaType<ListOfEntries>( "ListOfEntries" );
  qDebug() << qRegisterMetaType<MapStringImage>( "MapStringImage" );

  connect( ui.updateButton, SIGNAL( clicked() ), this, SLOT( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), settingsDialog, SLOT( show() ) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( filter, SIGNAL( escPressed() ), ui.statusEdit, SLOT( cancelEditing() ) );
  connect( &threadingEngine, SIGNAL( errorMessage( const QString& ) ), this, SLOT( popupError( const QString& ) ) );
  connect( &threadingEngine, SIGNAL( readyToDisplay( const ListOfEntries&, const MapStringImage& ) ), this, SLOT( display( const ListOfEntries&, const MapStringImage& ) ) );
  connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );
  connect( settingsDialog, SIGNAL( settingsOK() ), this, SLOT( saveConfig() ) );
  //updateTweets();
}

void MainWindow::popupMenu() {
  menu->exec( QCursor::pos() );
}

MainWindow::~MainWindow() {
  saveConfig();
  if ( filter ) {
    delete filter;
    filter = NULL;
  }
  if ( fm ) {
    delete fm;
    fm = NULL;
  }
}

void MainWindow::changeLabel() {
  ui.countdownLabel->setText( ui.statusEdit->isStatusClean() ? QString::number( STATUS_MAX_LEN ) : QString::number( STATUS_MAX_LEN - ui.statusEdit->text().length() ) );
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + tr( " characters left" ) );
}

void MainWindow::updateTweets() {
  ui.updateButton->setEnabled( false );
  threadingEngine.get( "http://twitter.com/statuses/friends_timeline.xml" );
}

void MainWindow::sendStatus() {
  QByteArray status( "status=" );
  status.append( ui.statusEdit->text().toUtf8() );
  status.append( "&source=qtwitter" );
  qDebug() << status;
  const QString path("http://twitter.com/statuses/update.xml");
  threadingEngine.post( path, status );
}

void MainWindow::resetStatus() {
  if ( ui.statusEdit->isStatusClean() ) {
    changeLabel();
  }
}

void MainWindow::resizeEvent( QResizeEvent *event ) {
  if ( model.rowCount() == 0 )
    return;

  QSize itemSize;
  int scrollBarMargin = ui.statusListView->verticalScrollBar()->size().width();
  for ( int i = 0; i < model.rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( ui.statusListView->indexWidget( model.indexFromItem( model.item(i) ) ) );
    aTweet->resize( event->size().width() - scrollBarMargin, aTweet->size().height() );
    itemSize = model.item(i)->sizeHint();
    itemSize.rwidth() += event->size().width() - event->oldSize().width();
    itemSize.rheight() = aTweet->size().height();
    model.item(i)->setSizeHint( itemSize );
  }
}

void MainWindow::display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash ) {
  model.clear();
  int scrollBarMargin = ui.statusListView->verticalScrollBar()->size().width();
  for ( int i = 0; i < entries.size(); i++ ) {
    QStandardItem *newItem = new QStandardItem();
    Tweet *newTweet = new Tweet( entries[i].name(), entries[i].text(), imagesHash[ entries[i].image() ], this );
    newTweet->resize( ui.statusListView->width() - scrollBarMargin, newTweet->size().height() );
    newItem->setSizeHint( newTweet->size() );
    model.appendRow( newItem );
    ui.statusListView->setIndexWidget( model.indexFromItem( newItem ), newTweet );
  }
  unlockState();
}

void MainWindow::unlockState() {
  ui.updateButton->setEnabled( true );
  if ( !ui.statusEdit->isEnabled() ) {
    ui.statusEdit->setEnabled( true );
    ui.statusEdit->initialize();
  }
}

void MainWindow::popupError( const QString &message ) {
  QMessageBox::information( this, tr("Error"), message );
  unlockState();
}

void MainWindow::loadConfig() {

#if defined Q_WS_X11 || defined Q_WS_MAC
  QSettings settings( "ayoy", "qTwitter" );
#endif
#if defined Q_WS_WIN
  QSettings settings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" );
#endif
  settings.beginGroup( "MainWindow" );
    resize( settings.value( "size", QSize(307, 245) ).toSize() );
    move( settings.value( "pos", QPoint(500, 300) ).toPoint() );
  settings.endGroup();
  settings.beginGroup( "General" );
    settingsDialog->ui.refreshCombo->setCurrentIndex( settings.value( "refresh" ).toInt() );
    settingsDialog->ui.languageCombo->setCurrentIndex( settings.value( "language", 0 ).toInt() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      settingsDialog->ui.proxyBox->setCheckState( (Qt::CheckState)settings.value( "enabled" ).toInt() );
      settingsDialog->ui.hostEdit->setText( settings.value( "host" ).toString() );
      settingsDialog->ui.portEdit->setText( settings.value( "port" ).toString() );
      settingsDialog->setProxy();
    settings.endGroup();
  settings.endGroup();

  settingsDialog->ui.hostEdit->setEnabled( (bool) settingsDialog->ui.proxyBox->checkState() );
  settingsDialog->ui.portEdit->setEnabled( (bool) settingsDialog->ui.proxyBox->checkState() );
}

void MainWindow::saveConfig() {

#if defined Q_WS_X11 || defined Q_WS_MAC
  QSettings settings( "ayoy", "qTwitter" );
#endif
#if defined Q_WS_WIN
  QSettings settings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" );
#endif
  qDebug() << size().width() << size().height();
  settings.beginGroup( "MainWindow" );
    settings.setValue( "size", size() );
    settings.setValue( "pos", pos() );
  settings.endGroup();
  settings.beginGroup( "General" );
    settings.setValue( "refresh", settingsDialog->ui.refreshCombo->currentIndex() );
    settings.setValue( "language", settingsDialog->ui.languageCombo->currentIndex() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      settings.setValue( "enabled", settingsDialog->ui.proxyBox->checkState() );
      settings.setValue( "host", settingsDialog->ui.hostEdit->text() );
      settings.setValue( "port", settingsDialog->ui.portEdit->text() );
    settings.endGroup();
  settings.endGroup();
}
