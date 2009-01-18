#include "mainwindow.h"
#include "tweet.h"

#include <QMenu>

MainWindow::MainWindow() : QWidget(), model( 0, 0, this )
{
  ui.setupUi( this );
  ui.countdownLabel->setToolTip( ui.countdownLabel->text() + tr( " characters left" ) );
  filter = new StatusFilter();
  fm = new QFontMetrics( ui.statusListView->font() );
  settingsDialog = new Settings( this );
  settingsDialog->loadConfig();
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

  connect( ui.updateButton, SIGNAL( clicked() ), this, SLOT( updateTweets() ) );
  connect( ui.settingsButton, SIGNAL( clicked() ), settingsDialog, SLOT( show() ) );
  connect( ui.statusEdit, SIGNAL( textChanged( QString ) ), this, SLOT( changeLabel() ) );
  connect( ui.statusEdit, SIGNAL( lostFocus() ), this, SLOT( resetStatus() ) );
  connect( filter, SIGNAL( enterPressed() ), this, SLOT( sendStatus() ) );
  connect( &threadingEngine, SIGNAL( errorMessage( const QString& ) ), this, SLOT( popupError( const QString& ) ) );
  connect( &threadingEngine, SIGNAL( readyToDisplay( const QList<Entry>&, const QMap<QString, QImage>& ) ), this, SLOT( display( const QList<Entry>&, const QMap<QString, QImage>& ) ) );
  connect( ui.statusListView, SIGNAL( contextMenuRequested() ), this, SLOT( popupMenu() ) );

  //updateTweets();
}

void MainWindow::popupMenu() {
  menu->exec( QCursor::pos() );
}

MainWindow::~MainWindow() {
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
  ui.countdownLabel->setText( QString::number( STATUS_MAX_LEN - ui.statusEdit->text().length() ) );
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
  if ( ui.statusEdit->text().length() == 0 )
  {
    ui.statusEdit->initialize();
    ui.countdownLabel->setText( QString::number(STATUS_MAX_LEN) );
  }
}

void MainWindow::resizeEvent( QResizeEvent *event ) {
  if ( model.rowCount() == 0 )
    return;

  QSize itemSize;

  for ( int i = 0; i < model.rowCount(); i++ ) {
    Tweet *aTweet = dynamic_cast<Tweet*>( ui.statusListView->indexWidget( model.indexFromItem( model.item(i) ) ) );
    aTweet->resize( event->size().width(), aTweet->size().height() );
    itemSize = model.item(i)->sizeHint();
    itemSize.rwidth() += event->size().width() - event->oldSize().width();
    itemSize.rheight() = aTweet->size().height();
    model.item(i)->setSizeHint( itemSize );
  }
}

void MainWindow::display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash ) {
  model.clear();
  for ( int i = 0; i < entries.size(); i++ ) {
    QStandardItem *newItem = new QStandardItem();
    Tweet *newTweet = new Tweet( entries[i].name(), entries[i].text(), imagesHash[ entries[i].image() ], this );
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

