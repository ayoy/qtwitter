/***************************************************************************
 *   Copyright (C) 2008-2009 by Dominik Kapusta       <d@ayoy.net>         *
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


#include <QPushButton>
#include <QCheckBox>
#include <QPoint>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProcess>
#include <QSettings>
#include "settings.h"
#include "core.h"
#include "mainwindow.h"
#include "twitpicview.h"
#include "twitteraccountsmodel.h"
#include "twitteraccountsdelegate.h"
#include "urlshortener.h"

const QString ConfigFile::APP_VERSION = "0.6.0";

ConfigFile settings;

ConfigFile::ConfigFile():
#if defined Q_WS_MAC
QSettings( QSettings::defaultFormat(), QSettings::UserScope, "ayoy.net", "qTwitter" )
#elif defined Q_WS_WIN
QSettings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" )
#else
QSettings( QSettings::defaultFormat(), QSettings::UserScope, "ayoy", "qTwitter" )
#endif
{
  if ( value( "General/version", QString() ).toString().isNull() )
    convertSettings();
}

QString ConfigFile::pwHash( const QString &text )
{
  QString newText = text;
  for (unsigned int i = 0, textLength = text.length(); i < textLength; ++i)
    newText[i] = QChar(text[i].unicode() ^ i ^ 1);
  return newText;
}

void ConfigFile::deleteTwitterAccount( int id, int rowCount )
{
  beginGroup( "TwitterAccounts" );
  if ( id < rowCount ) {
    for (int i = id; i < rowCount; i++ ) {
      setValue( QString( "%1/enabled" ).arg(i), value( QString( "%1/enabled" ).arg(i+1) ) );
      setValue( QString( "%1/login" ).arg(i), value( QString( "%1/login" ).arg(i+1) ) );
      setValue( QString( "%1/password" ).arg(i), value( QString( "%1/password" ).arg(i+1) ) );
      setValue( QString( "%1/directmsgs" ).arg(i), value( QString( "%1/directmsgs" ).arg(i+1) ) );
    }
  }
  remove( QString::number( rowCount ) );
  endGroup();
}

void ConfigFile::convertSettings()
{
  setValue( "General/version", ConfigFile::APP_VERSION );
  if ( contains( "General/username" ) ) {
    setValue( "TwitterAccounts/0/enabled", true );
    setValue( "TwitterAccounts/0/login", value( "General/username", "<empty>" ).toString() );
    setValue( "TwitterAccounts/0/password", value( "General/password", "" ).toString() );
    setValue( "TwitterAccounts/0/directmsgs", value( "General/directMessages", false ).toBool() );
  }
  setValue( "TwitterAccounts/publicTimeline", true );
  if ( value( "General/timeline", false ).toBool() ) {
    setValue( "TwitterAccounts/currentModel", 1 );
  }
  setValue( "General/language", "en" );
  remove( "General/username" );
  remove( "General/password" );
  remove( "General/directMessages" );
  remove( "General/timeline" );
}

const ThemeInfo Settings::STYLESHEET_CARAMEL =  ThemeInfo( QString( "Caramel" ),
                                                           ThemeData( ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(23, 14, 40, 255), stop:0.0150754 rgba(23, 14, 40, 255), stop:1 rgba(112, 99, 37, 255)); border-width: 3px; border-style: outset; border-color: rgb(219, 204, 56); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal}" ),
                                                                                    QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                    QColor( 51, 51, 51 ) ),
                                                                      ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(23, 14, 40, 255), stop:0.2 rgba(67, 67, 38, 255), stop:1 rgba(112, 99, 37, 255)); border-width: 3px; border-style: outset; border-color: rgb(255, 255, 158); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal}" ),
                                                                                    QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                    QColor( 51, 51, 51 ) ),
                                                                      ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(23, 14, 40, 255), stop:0.8 rgba(67, 67, 38, 255), stop:1 rgba(112, 99, 37, 255)); border-width: 3px; border-style: outset; border-color: rgb(119, 102, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-style: normal}" ),
                                                                                    QString( "a { color: rgb(153, 146, 38); }" ),
                                                                                    QColor( 51, 51, 51 ) ) ) );
const ThemeInfo Settings::STYLESHEET_COCOA   = ThemeInfo( QString( "Cocoa" ),
                                                          ThemeData( ThemeElement( QString( "QFrame { background-color: rgb(204, 153, 102); border-width: 3px; border-style: outset; border-color: rgb(51, 51, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(51, 51, 0); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(51, 51, 0); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 40); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(234, 183, 132); border-width: 3px; border-style: outset; border-color: rgb(102, 102, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(71, 71, 20); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(71, 71, 0); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 40); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(153, 102, 51); border-width: 3px; border-style: outset; border-color: rgb(20, 20, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(31, 31, 0); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(31, 31, 0); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(215, 208, 0); }" ),
                                                                                   QColor( 51, 51, 51 ) ) ) );
const ThemeInfo Settings::STYLESHEET_GRAY    = ThemeInfo( QString( "Gray" ),
                                                          ThemeData( ThemeElement( QString( "QFrame { background-color: rgb(102, 102, 102); border-width: 3px; border-style: outset; border-color: rgb(0, 0, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(133, 133, 133); border-width: 3px; border-style: outset; border-color: rgb(204, 204, 204); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(51, 51, 51); border-width: 3px; border-style: outset; border-color: rgb(0, 0, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(215, 208, 100); }" ),
                                                                                   QColor( 51, 51, 51 ) ) ) );
const ThemeInfo Settings::STYLESHEET_GREEN   = ThemeInfo( QString( "Green" ),
                                                          ThemeData( ThemeElement( QString( "QFrame { background-color: rgb(102, 153, 0); border-width: 3px; border-style: outset; border-color: rgb(0, 72, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(132, 183, 30); border-width: 3px; border-style: outset; border-color: rgb(0, 92, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(255, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 51, 51, 51 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(51, 102, 0); border-width: 3px; border-style: outset; border-color: rgb(0, 32, 0); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(153, 153, 153); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(153, 146, 38); }" ),
                                                                                   QColor( 51, 51, 51 ) ) ) );
const ThemeInfo Settings::STYLESHEET_PURPLE  = ThemeInfo( QString( "Purple" ),
                                                          ThemeData( ThemeElement( QString( "QFrame { background-color: rgb(153, 102, 204); border-width: 3px; border-style: outset; border-color: rgb(153, 153, 204); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(153, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(153, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 62, 21, 113 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(183, 132, 234); border-width: 3px; border-style: outset; border-color: rgb(173, 173, 224); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(173, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(173, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(255, 248, 140); }" ),
                                                                                   QColor( 62, 21, 113 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: rgb(102, 51, 153); border-width: 3px; border-style: outset; border-color: rgb(113, 113, 164); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(102, 255, 255); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(102, 255, 255); border-width: 0px; border-style: normal; }" ),
                                                                                   QString( "a { color: rgb(153, 146, 38); }" ),
                                                                                   QColor( 62, 21, 113 ) ) ) );
const ThemeInfo Settings::STYLESHEET_SKY     = ThemeInfo( QString( "Sky" ),
                                                          ThemeData( ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 187, 218, 255), stop:0.5 rgba(180, 209, 236, 255), stop:1 rgba(222, 231, 255, 255)); border-width: 3px; border-style: outset; border-color: rgb(163, 199, 215); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(0, 60, 196); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(0, 60, 196); border-width: 0px; border-style: normal }" ),
                                                                                   QString( "a { color: rgb(0, 0, 204); }" ),
                                                                                   QColor( 184, 202, 215 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(139, 187, 218, 255), stop:0.2 rgba(180, 209, 236, 255), stop:1 rgba(222, 231, 255, 255)); border-width: 3px; border-style: outset; border-color: rgb(255, 255, 255); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(0, 90, 226); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(0, 90, 226); border-width: 0px; border-style: normal }" ),
                                                                                   QString( "a { color: rgb(0, 0, 255); }" ),
                                                                                   QColor( 184, 202, 215 ) ),
                                                                     ThemeElement( QString( "QFrame { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(89, 137, 168, 255), stop:0.8 rgba(180, 209, 236, 255), stop:1 rgba(222, 231, 255, 255)); border-width: 3px; border-style: outset; border-color: rgb(102, 137, 153); border-radius: 12px} QLabel { background-color: rgba(255, 255, 255, 0); color: rgb(0, 20, 156); border-width: 0px; border-radius: 0px } QTextBrowser { background-color: rgba(255, 255, 255, 0); color: rgb(0, 20, 156); border-width: 0px; border-style: normal }" ),
                                                                                   QString( "a { color: rgb(0, 0, 153); }" ),
                                                                                   QColor( 184, 202, 215 ) ) ) );

Settings::Settings( MainWindow *mainwinSettings, Core *coreSettings, TwitPicView *twitpicviewSettings, QWidget *parent ) :
    QDialog( parent ),
    mainWindow( mainwinSettings ),
    core( coreSettings ),
    twitPicView( twitpicviewSettings )
{  
  qApp->installTranslator( &translator );

  ui.setupUi( this );
  ui.languageCombo->setItemData( 0, "en" );
  accountsModel = qobject_cast<TwitterAccountsModel*>( core->getTwitterAccountsModel() );
  if ( accountsModel ) {
    ui.usersView->setModel( accountsModel );
  }
  ui.usersView->setItemDelegate( new TwitterAccountsDelegate( QList<int>() << 0 << 3, this ) );

  ui.usersView->hideColumn( 2 );
  ui.usersView->setColumnWidth( 0, (int)(ui.usersView->width() * 0.5 ));
  ui.usersView->setColumnWidth( 1, (int)(ui.usersView->width() * 1.0 ));
  ui.usersView->setColumnWidth( 3, (int)(ui.usersView->width() * 0.2 ));

  connect( ui.usersView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(fillAccountEditor(QModelIndex,QModelIndex)) );
  connect( ui.addAccountButton, SIGNAL(clicked()), this, SLOT(addTwitterAccount()));
  connect( ui.deleteAccountButton, SIGNAL(clicked()), this, SLOT(deleteTwitterAccount()));
  connect( ui.accountEnabledCheckBox, SIGNAL(clicked(bool)), this, SLOT(setTwitterAccountEnabled(bool)) );
  connect( ui.accountLoginEdit, SIGNAL(textEdited(QString)), this, SLOT(setTwitterAccountLogin(QString)) );
  connect( ui.accountPasswordEdit, SIGNAL(textEdited(QString)), this, SLOT(setTwitterAccountPassword(QString)) );
  connect( ui.accountDMCheckBox, SIGNAL(clicked(bool)), this, SLOT(setTwitterAccountDM(bool)) );
  connect( ui.publicTimelineCheckBox, SIGNAL(clicked(bool)), this, SLOT(setPublicTimelineEnabled(bool)) );

  themes.insert( STYLESHEET_CARAMEL.first, STYLESHEET_CARAMEL.second);
  themes.insert( STYLESHEET_COCOA.first,   STYLESHEET_COCOA.second);
  themes.insert( STYLESHEET_GRAY.first,    STYLESHEET_GRAY.second);
  themes.insert( STYLESHEET_GREEN.first,   STYLESHEET_GREEN.second);
  themes.insert( STYLESHEET_PURPLE.first,  STYLESHEET_PURPLE.second);
  themes.insert( STYLESHEET_SKY.first,     STYLESHEET_SKY.second);

  for (int i = 0; i < themes.keys().size(); ++i ) {
    ui.colorBox->addItem( themes.keys()[i] );
  }

#ifdef Q_WS_X11
  QHBoxLayout *hlayout = new QHBoxLayout;

  useCustomBrowserCheckBox = new QCheckBox( tr( "Use custom web browser" ), ui.networkTab );
  selectBrowserEdit = new QLineEdit( ui.networkTab );
  selectBrowserButton = new QPushButton( tr( "Browse" ), ui.networkTab );

  hlayout->addWidget(selectBrowserEdit);
  hlayout->addWidget(selectBrowserButton);
  ui.verticalLayout_2->addWidget(useCustomBrowserCheckBox);
  ui.verticalLayout_2->addLayout(hlayout);

  selectBrowserEdit->setEnabled( false );
  selectBrowserButton->setEnabled( false );

  connect( useCustomBrowserCheckBox, SIGNAL(toggled(bool)), selectBrowserEdit, SLOT(setEnabled(bool)) );
  connect( useCustomBrowserCheckBox, SIGNAL(toggled(bool)), selectBrowserButton, SLOT(setEnabled(bool)) );
  connect( selectBrowserButton, SIGNAL(clicked()), this, SLOT(setBrowser()) );
#endif

  connect( ui.buttonBox->button( QDialogButtonBox::Apply ), SIGNAL(clicked()), this, SLOT(saveConfig()) );
  connect( ui.languageCombo, SIGNAL( currentIndexChanged( int )), this, SLOT( switchLanguage( int ) ) );
  connect( ui.colorBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeTheme(QString)) );
  createLanguageMenu();
  createUrlShortenerMenu();
  ui.portEdit->setValidator( new QIntValidator( 1, 65535, this ) );
  loadConfig();
}

Settings::~Settings() {}

void Settings::loadConfig( bool dialogRejected )
{
  settings.beginGroup( "General" );
    ui.refreshCombo->setCurrentIndex( settings.value( "refresh-index", 3 ).toInt() );
    ui.languageCombo->setCurrentIndex( ui.languageCombo->findData( settings.value( "language", QLocale::system().name().left( 2 ) ) ) );
    ui.urlShortenerCombo->setCurrentIndex( ui.urlShortenerCombo->findData( settings.value( "url-shortener", UrlShortener::SHORTENER_ISGD ).toInt() ) );
    ui.confirmDeletionBox->setChecked( settings.value( "confirmTweetDeletion", true ).toBool() );
    ui.notificationsBox->setChecked( settings.value( "notifications", true ).toBool() );
  settings.endGroup();
  settings.beginGroup( "TwitterAccounts" );
  if ( !dialogRejected ) {
    accountsModel->clear();
      for ( int i = 0; i < settings.childGroups().count(); i++ ) {
        accountsModel->insertRow(i);
        accountsModel->account(i).isEnabled = settings.value( QString( "%1/enabled" ).arg(i), false ).toBool();
        accountsModel->account(i).login = settings.value( QString( "%1/login" ).arg(i), "" ).toString();
        accountsModel->account(i).password = settings.pwHash( settings.value( QString( "%1/password" ).arg(i), "" ).toString() );
        accountsModel->account(i).directMessages = settings.value( QString( "%1/directmsgs" ).arg(i), false ).toBool();
      }
      if ( ui.usersView->model()->rowCount() <= 1 ) {
        ui.deleteAccountButton->setEnabled( false );
      } else {
        ui.deleteAccountButton->setEnabled( true );
      }
  }
  ui.publicTimelineCheckBox->setChecked( settings.value( "publicTimeline", false ).toBool() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      ui.proxyBox->setChecked( settings.value( "enabled" ).toBool() );
      ui.hostEdit->setText( settings.value( "host" ).toString() );
      ui.portEdit->setText( settings.value( "port" ).toString() );
    settings.endGroup();
#ifdef Q_WS_X11
    useCustomBrowserCheckBox->setChecked( settings.value( "customBrowser", Qt::Unchecked ).toBool() );
    selectBrowserEdit->setText( settings.value( "browser" ).toString() );
#endif
  settings.endGroup();
  settings.beginGroup( "Appearance" );
    ui.tweetCountBox->setValue( settings.value( "tweet count", 25 ).toInt() );
    ui.colorBox->setCurrentIndex( settings.value( "color scheme", 3 ).toInt() );
  settings.endGroup();

  ui.hostEdit->setEnabled( (bool) ui.proxyBox->checkState() );
  ui.portEdit->setEnabled( (bool) ui.proxyBox->checkState() );

  if ( !dialogRejected ) {
    settings.beginGroup( "MainWindow" );
    mainWindow->resize( settings.value( "size", QSize(300, 400) ).toSize() );
    QPoint offset( settings.value( "pos", QPoint(500,500) ).toPoint() );
    if ( QApplication::desktop()->width() < offset.x() + settings.value( "size" ).toSize().width() ) {
      offset.setX( QApplication::desktop()->width() - settings.value( "size" ).toSize().width() );
    }
    if ( QApplication::desktop()->height() < offset.y() + settings.value( "size" ).toSize().height() ) {
      offset.setY( QApplication::desktop()->height() - settings.value( "size" ).toSize().height() );
    }
    mainWindow->move( offset );
    settings.endGroup();
    offset = settings.value( "SettingsWindow/pos", QPoint(500,500) ).toPoint();
    if ( QApplication::desktop()->width() < offset.x() + size().width() ) {
      offset.setX( QApplication::desktop()->width() - size().width() );
    }
    if ( QApplication::desktop()->height() < offset.y() + size().height() ) {
      offset.setY( QApplication::desktop()->height() - size().height() );
    }
    move( offset );
    applySettings();
  }
  qDebug() << "settings loaded and applied";
}

void Settings::setProxy()
{
  if ( ui.proxyBox->isChecked() ) {
    proxy.setType( QNetworkProxy::HttpProxy );
    proxy.setHostName( ui.hostEdit->text() );
    proxy.setPort( ui.portEdit->text().toInt() );
  } else {
    proxy.setType( QNetworkProxy::NoProxy );
  }
  QNetworkProxy::setApplicationProxy( proxy );
}

void Settings::saveConfig( int quitting )
{
  settings.beginGroup( "MainWindow" );
    settings.setValue( "size", mainWindow->size() );
    settings.setValue( "pos", mainWindow->pos() );
  settings.endGroup();
  settings.setValue( "SettingsWindow/pos", pos() );
  settings.beginGroup( "General" );
    settings.setValue( "refresh-index", ui.refreshCombo->currentIndex() );
    settings.setValue( "refresh-value", ui.refreshCombo->currentText() );
    settings.setValue( "language", ui.languageCombo->itemData( ui.languageCombo->currentIndex() ).toString() );
    settings.setValue( "url-shortener", ui.urlShortenerCombo->itemData( ui.urlShortenerCombo->currentIndex() ).toInt() );
    settings.setValue( "confirmTweetDeletion", ui.confirmDeletionBox->isChecked() );
    settings.setValue( "notifications", ui.notificationsBox->isChecked() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      settings.setValue( "enabled", ui.proxyBox->isChecked() );
      settings.setValue( "host", ui.hostEdit->text() );
      settings.setValue( "port", ui.portEdit->text() );
    settings.endGroup();
#ifdef Q_WS_X11
    if ( useCustomBrowserCheckBox->isChecked() && selectBrowserEdit->text().isEmpty() ) {
      useCustomBrowserCheckBox->setChecked( false );
    }
    settings.setValue( "customBrowser", useCustomBrowserCheckBox->isChecked() );
    settings.setValue( "browser", selectBrowserEdit->text() );
#endif
  settings.endGroup();
  settings.beginGroup( "Appearance" );
    settings.setValue( "tweet count", ui.tweetCountBox->value() );
    settings.setValue( "color scheme", ui.colorBox->currentIndex() );
  settings.endGroup();

  if ( !quitting ) {
    applySettings();
    qDebug() << "settings applied and saved";
  }
}

void Settings::show()
{
  ui.tabs->setCurrentIndex( 0 );
  QDialog::show();
  adjustSize();
  if ( accountsModel->index( ui.usersView->currentIndex().row(), 0 ).isValid() ) {
    TwitterAccount &account = accountsModel->account( accountsModel->index( ui.usersView->currentIndex().row(), 0 ).row() );
    ui.accountEnabledCheckBox->setChecked( account.isEnabled );
  }
}

void Settings::accept()
{
  saveConfig();
  QDialog::accept();
}

void Settings::reject()
{
  loadConfig( true );
  QDialog::reject();
}

void Settings::switchLanguage( int index )
{
  QString locale = ui.languageCombo->itemData( index ).toString();
  QString qmPath( ":/translations" );
  qDebug() << "switching language to" << locale << "from" << qmPath;
  translator.load( "qtwitter_" + locale, qmPath );
  retranslateUi();
//  ui.retranslateUi(this);
  mainWindow->retranslateUi();
  core->retranslateUi();
 // ui.tabs->adjustSize();
  adjustSize();
}

void Settings::fillAccountEditor( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED(previous);

  ui.accountEnabledCheckBox->setEnabled( true );
  ui.accountLoginLabel->setEnabled( true );
  ui.accountLoginEdit->setEnabled( true );
  ui.accountPasswordLabel->setEnabled( true );
  ui.accountPasswordEdit->setEnabled( true );
  ui.accountDMCheckBox->setEnabled( true );

  QModelIndex rootIndex;
  int row = current.row();
  rootIndex = current.sibling( row, 0 );
  ui.accountEnabledCheckBox->setChecked( rootIndex.data().toBool() );
  ui.accountLoginEdit->setText( rootIndex.sibling( row, 1 ).data().toString() );
  ui.accountPasswordEdit->setText( rootIndex.sibling( row, 2 ).data().toString() );
  ui.accountDMCheckBox->setChecked( rootIndex.sibling( row, 3 ).data().toBool() );
}

void Settings::addTwitterAccount()
{
  accountsModel->insertRow( accountsModel->rowCount() );
  ui.usersView->setCurrentIndex( accountsModel->index( accountsModel->rowCount() - 1, 0 ) );
  ui.deleteAccountButton->setEnabled( true );
  ui.accountLoginEdit->setFocus();
  ui.accountLoginEdit->selectAll();
  settings.beginGroup( QString( "TwitterAccounts/%1" ).arg( accountsModel->rowCount() - 1 ) );
    settings.setValue( "enabled", true );
    //: This is for newly created account - when the login isn't given yet
    settings.setValue( "login", tr( "<empty>" ) );
    settings.setValue( "password", "" );
    settings.setValue( "directmsgs", false );
  settings.endGroup();
}

void Settings::deleteTwitterAccount()
{
  if ( !ui.usersView->selectionModel()->currentIndex().isValid() )
    return;
  int row = ui.usersView->selectionModel()->currentIndex().row();
  accountsModel->removeRow( row );
  settings.deleteTwitterAccount( row, accountsModel->rowCount() );
  if ( accountsModel->rowCount() <= 1 ) {
    ui.deleteAccountButton->setEnabled( false );
  } else {
    ui.deleteAccountButton->setEnabled( true );
  }
}

void Settings::setTwitterAccountEnabled( bool state )
{
  if ( !ui.usersView->selectionModel()->currentIndex().isValid() )
    return;
  accountsModel->account( ui.usersView->currentIndex().row() ).isEnabled = state;
  ui.usersView->update( accountsModel->index( ui.usersView->currentIndex().row(), 0, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/enabled").arg( ui.usersView->currentIndex().row() ), state );
}

void Settings::setTwitterAccountLogin( const QString &login )
{
  if ( !ui.usersView->selectionModel()->currentIndex().isValid() )
    return;
  TwitterAccount &account = accountsModel->account( ui.usersView->currentIndex().row() );
  account.login = login;
  ui.usersView->update( accountsModel->index( ui.usersView->currentIndex().row(), 1, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/login").arg( ui.usersView->currentIndex().row() ), login );
}

void Settings::setTwitterAccountPassword( const QString &password )
{
  if ( !ui.usersView->selectionModel()->currentIndex().isValid() )
    return;
  TwitterAccount &account = accountsModel->account( ui.usersView->currentIndex().row() );
  account.password = password;
  settings.setValue( QString("TwitterAccounts/%1/password").arg( ui.usersView->currentIndex().row() ), settings.pwHash( password ) );
}

void Settings::setTwitterAccountDM( bool state )
{
  if ( !ui.usersView->selectionModel()->currentIndex().isValid() )
    return;
  TwitterAccount &account = accountsModel->account( ui.usersView->currentIndex().row() );
  account.directMessages = state;
  ui.usersView->update( accountsModel->index( ui.usersView->currentIndex().row(), 3, QModelIndex() ) );
  settings.setValue( QString("TwitterAccounts/%1/directmsgs").arg( ui.usersView->currentIndex().row() ), state );
}

void Settings::setPublicTimelineEnabled( bool state )
{
  settings.setValue( "TwitterAccounts/publicTimeline", state );
}

void Settings::changeTheme( const QString &theme )
{
  mainWindow->changeListBackgroundColor( themes.value( theme ).unread.listBackgroundColor );
  core->setModelTheme( themes.value( theme ) );
}

void Settings::retranslateUi()
{
  this->setWindowTitle( tr("Settings") );
  ui.tabs->setTabText( 0, tr( "General" ) );
  ui.refreshLabel->setText( tr("Refresh every (mins):") );
  ui.languageLabel->setText( tr("Language:") );
  ui.shortenLabel->setText( tr("Shorten links via:") );
  ui.notificationsBox->setText( tr("Show tray notifications") );
  ui.confirmDeletionBox->setText( tr("Confirm messages deletion") );
  ui.tabs->setTabText( 1, tr( "Twitter" ) );
  ui.accountGroupBox->setTitle( tr( "Account" ) );
  ui.accountEnabledCheckBox->setText( tr( "Enabled" ) );
  ui.accountLoginLabel->setText( tr( "Username:" ) );
  ui.accountPasswordLabel->setText( tr( "Password:" ) );
  ui.accountDMCheckBox->setText( tr( "download direct messages" ) );
  ui.publicTimelineCheckBox->setText( tr( "include public timeline" ) );
  ui.tabs->setTabText( 2, tr( "Network" ) );
  ui.proxyBox->setText( tr( "Use HTTP &proxy" ) );
  ui.hostLabel->setText( tr( "Host:" ) );
  ui.portLabel->setText( tr( "Port:" ) );
  ui.tabs->setTabText( 3, tr( "Appearance" ) );
  ui.tweetCountLabel->setText( tr( "Tweet count:" ) );
  ui.colorLabel->setText( tr( "Color scheme:" ) );
#ifdef Q_WS_X11
  useCustomBrowserCheckBox->setText( tr( "Use custom web browser" ) );
  selectBrowserButton->setText( tr( "Browse" ) );
#endif
  ui.buttonBox->button( QDialogButtonBox::Apply )->setText( tr( "Apply" ) );
  ui.buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "Cancel" ) );
  ui.buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "OK" ) );
  update();
//  adjustSize();
//  updateGeometry();
}

#ifdef Q_WS_X11
void Settings::setBrowser()
{
  QRegExp rx( ";HOME=(.+);", Qt::CaseSensitive );
  rx.setMinimal( true );
  rx.indexIn( QProcess::systemEnvironment().join( ";" ) );
  selectBrowserEdit->setText( QFileDialog::getOpenFileName( this, tr( "Select your browser executable" ), rx.cap( 1 ), tr( "All files (*)") ) );
}
#endif

void Settings::applySettings()
{
  setProxy();
  core->applySettings();
  mainWindow->setupTwitterAccounts( accountsModel->getAccounts(), ui.publicTimelineCheckBox->isChecked() );
  twitPicView->setupTwitterAccounts( accountsModel->getAccounts() );
  changeTheme( ui.colorBox->currentText() );
#ifdef Q_WS_X11
  if ( useCustomBrowserCheckBox->isChecked() ) {
    QDesktopServices::setUrlHandler( "http", core, "openBrowser");
    core->setBrowserPath( this->selectBrowserEdit->text() );
  } else {
    QDesktopServices::unsetUrlHandler( "http" );
    core->setBrowserPath( QString() );
  }
#endif
}

void Settings::createLanguageMenu()
{
  QDir qmDir( ":/translations" );
  QStringList fileNames = qmDir.entryList(QStringList("qtwitter_*.qm"));
  for (int i = 0; i < fileNames.size(); ++i) {
    QString locale = fileNames[i];
    locale.remove(0, locale.indexOf('_') + 1);
    locale.chop(3);

    QTranslator translator;
    translator.load(fileNames[i], qmDir.absolutePath());
    QString language = translator.translate("Settings", "English");
    qDebug() << "adding language" << language << ", locale" << locale;
    ui.languageCombo->addItem( language, locale );
  }
  QString systemLocale = QLocale::system().name();
  systemLocale.chop(3);
  qDebug() << systemLocale << ui.languageCombo->findData( systemLocale );
  ui.languageCombo->setCurrentIndex( ui.languageCombo->findData( systemLocale ) );
}

void Settings::createUrlShortenerMenu()
{
  ui.urlShortenerCombo->addItem( "is.gd", UrlShortener::SHORTENER_ISGD );
  ui.urlShortenerCombo->addItem( "MetaMark", UrlShortener::SHORTENER_METAMARK );
  ui.urlShortenerCombo->addItem( "tinyarro.ws", UrlShortener::SHORTENER_TINYARROWS );
  ui.urlShortenerCombo->addItem( "TinyURL", UrlShortener::SHORTENER_TINYURL );
  ui.urlShortenerCombo->addItem( "tr.im", UrlShortener::SHORTENER_TRIM );
}

/*! \struct ThemeElement
    \brief A struct containing customization data for themed UI elements.

    This struct provides the stylesheet data and palette colors of the customizable
    UI elements. Its elements are loaded and dynamically applied when selected from
    the settings dialog.
*/

/*! \var QString ThemeElement::styleSheet
    Tweet class stylesheet.
*/

/*! \var QString ThemeElement::linkColor
    A color for links in status text field in Tweet class.
*/

/*! \var QColor ThemeElement::listBackgroundColor
    A color for MainWindow class list view background.
*/

/*! \fn ThemeElement::ThemeElement()
    Creates an empty ThemeElement struct.
*/

/*! \fn ThemeElement::ThemeElement( const QString &newStyleSheet, const QString &newLinkColor, const QColor &newListBackgroundColor )
    Creates the new instance of a struct and initializes its fields according to given parameters.
*/

/*! \fn ThemeElement::ThemeElement( const ThemeElement &other )
    A copy constructor.
*/

/*! \struct ThemeData
    \brief A struct holding complete theme settings.

    This struct consists of three ThemeElement members, for three states of
    a Tweet (Unread, Active and Read).
*/

/*! \var ThemeElement ThemeData::unread
    Theme settings for the Unread state.
*/

/*! \var ThemeElement ThemeData::active
    Theme settings for the Active state.
*/

/*! \var ThemeElement ThemeData::read
    Theme settings for the Read state.
*/

/*! \fn ThemeData::ThemeData()
    Creates an empty ThemeData struct.
*/

/*! \fn ThemeData::ThemeData( const ThemeElement &_unread, const ThemeElement &_active, const ThemeElement &_read )
    Creates the new instance of a struct and initializes its fields according to given parameters.
*/

/*! \typedef QPair<QString,ThemeData> ThemeInfo
    Type consisting of a pair of theme name stored as a string and theme data stored as a ThemeData struct.
*/

/*! \class Settings
    \brief A class defining the settings dialog and data.

    This class is a centralized repository of the application's settings.
    It contains the UI elements of a settings dialog, as well as the themes' data
    for customizing applicatoin's UI.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_CARAMEL
    Caramel theme info.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_COCOA
    Cocoa theme info.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_GRAY
    Gray theme info.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_GREEN
    Green theme info.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_PURPLE
    Purple theme info.
*/

/*! \var static const Settings::ThemeInfo STYLESHEET_SKY
    Sky theme info.
*/

/*! \fn Settings::Settings( TweetModel *tweetModel, MainWindow *mainwinSettings, Core *coreSettings, QWidget *parent = 0 )
    A default constructor. Creates the Settings class instance with the
    given \a parent and sets its values according to given parameters.
    \param tweetModel A pointer to TweetModel class instance, for applying its settings.
    \param mainwinSettings A pointer to MainWindow class instance, for applying its settings.
    \param coreSettings A pointer to Core class instance, for applying its settings.
    \param parent A class instance's parent.
*/

/*! \fn Settings::~Settings()
    A destructor.
*/

/*! \fn void Settings::loadConfig( bool dialogRejected = false )
    Loads the configuration from a settings file at application startup and whenever
    the settings dialog is rejected. Skips restoring dialog's and main window's size
    and position when \a dialogRejected is false.
    \param dialogRejected Indicates wether the method is invoked because of the
                          settings dialog being rejected or from other reason.
    \sa saveConfig(), reject()
*/

/*! \fn void Settings::setProxy()
    Sets application proxy according to dialog fields edited by User.
*/

/*! \fn void Settings::saveConfig( int quitting = 0 )
    Saves the configuration to a settings file. The saving is done when application
    quits and whenever the settings dialog is accepted or Apply button is pressed.
    Calls applySettings() when application isn't quitting at the moment.
    \param quitting When value is other than 0, method assumes that application
           is now quitting and doesn't apply the saved configuration. This avoids
           popping up a login window when quitting in some situations.
    \sa loadConfig(), accept()
*/

/*! \fn void Settings::show()
    Method reimplemented to always switch to the first settings' tab.
*/

/*! \fn void Settings::accept()
    Method reimplemented to save config upon accepting a dialog.
    \sa reject()
*/

/*! \fn void Settings::reject()
    Method reimplemented to load saved config upon rejecting a dialog.
    \sa accept()
*/

/*! \fn void Settings::switchLanguage( int index )
    Switches application's UI language to specified by \a index. Invokes
    retranslateUi() for all the widgets, including self.
    \param index Indicates a requested language by a language combo box's current index.
*/

/*! \fn void Settings::slotPublicTimelineSyncChanged( bool isEnabled )
    Applies timeline setting change made from a login window to a settings file.
    \param isEnabled Indicates whether public timeline downloading is enabled now.
*/

/*! \fn void Settings::setAuthDataInDialog( const QAuthenticator &authData )
    Applies authentication data change made from a login window to a settings dialog.
    \param authData Authentication data to be saved.
*/
