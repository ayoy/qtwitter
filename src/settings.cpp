#include "settings.h"
#include "mainwindow.h"
#include <QPushButton>
#include <QPoint>
#include <QDesktopWidget>

Settings::Settings( MainWindow *mainwinSettings, LoopedSignal *loopSettings, Core *coreSettings, QWidget *parent ) :
    QDialog( parent ),
    mainWindow( mainwinSettings ),
    loopedSignal( loopSettings ),
    core( coreSettings )
{
  qApp->installTranslator( &translator );
  ui.setupUi( this );
  connect( ui.languageCombo, SIGNAL( currentIndexChanged( int )), this, SLOT( switchLanguage( int ) ) );
  createLanguageMenu();
  QIntValidator *portValidator = new QIntValidator( 1, 65535, this );
  ui.portEdit->setValidator( portValidator );
  loadConfig();
}

Settings::~Settings() {}

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

void Settings::loadConfig( bool dialogRejected )
{

#if defined Q_WS_X11 || defined Q_WS_MAC
  QSettings settings( "ayoy", "qTwitter" );
#endif
#if defined Q_WS_WIN
  QSettings settings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" );
#endif
  settings.beginGroup( "General" );
    ui.refreshCombo->setCurrentIndex( settings.value( "refresh" ).toInt() );
    ui.languageCombo->setCurrentIndex( settings.value( "language", 0 ).toInt() );
    ui.radioFriends->setChecked( settings.value( "timeline", true ).toBool() );
    ui.radioPublic->setChecked( !ui.radioFriends->isChecked() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      ui.proxyBox->setCheckState( (Qt::CheckState)settings.value( "enabled" ).toInt() );
      ui.hostEdit->setText( settings.value( "host" ).toString() );
      ui.portEdit->setText( settings.value( "port" ).toString() );
    settings.endGroup();
  settings.endGroup();

  ui.hostEdit->setEnabled( (bool) ui.proxyBox->checkState() );
  ui.portEdit->setEnabled( (bool) ui.proxyBox->checkState() );

  if ( !dialogRejected ) {
    settings.beginGroup( "MainWindow" );
    mainWindow->resize( settings.value( "size", QSize(307, 245) ).toSize() );
    QPoint offset( settings.value( "pos" ).toPoint() );
    if ( QApplication::desktop()->width() < offset.x() + settings.value( "size" ).toSize().width() ) {
      offset.setX( QApplication::desktop()->width() - settings.value( "size" ).toSize().width() );
    }
    if ( QApplication::desktop()->height() < offset.y() + settings.value( "size" ).toSize().height() ) {
      offset.setY( QApplication::desktop()->height() - settings.value( "size" ).toSize().height() );
    }
    mainWindow->move( offset );
    settings.endGroup();
    applySettings();
  }
  qDebug() << "settings loaded and applied";
}

void Settings::saveConfig()
{

#if defined Q_WS_X11 || defined Q_WS_MAC
  QSettings settings( "ayoy", "qTwitter" );
#endif
#if defined Q_WS_WIN
  QSettings settings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" );
#endif
  settings.beginGroup( "MainWindow" );
    settings.setValue( "size", mainWindow->size() );
    settings.setValue( "pos", mainWindow->pos() );
  settings.endGroup();
  settings.beginGroup( "General" );
    settings.setValue( "refresh", ui.refreshCombo->currentIndex() );
    settings.setValue( "language", ui.languageCombo->currentIndex() );
    settings.setValue( "timeline", ui.radioFriends->isChecked() );
  settings.endGroup();
  settings.beginGroup( "Network" );
    settings.beginGroup( "Proxy" );
      settings.setValue( "enabled", ui.proxyBox->checkState() );
      settings.setValue( "host", ui.hostEdit->text() );
      settings.setValue( "port", ui.portEdit->text() );
    settings.endGroup();
  settings.endGroup();

  applySettings();
  qDebug() << "settings applied and saved";
}

void Settings::applySettings()
{
  setProxy();
  loopedSignal->setPeriod( ui.refreshCombo->currentText().toInt() * 60 );
  core->setDownloadPublicTimeline( ui.radioPublic->isChecked() );
  core->setAuthData( ui.userNameEdit->text(), ui.passwordEdit->text() );
}

void Settings::setAuthDataInDialog( const QAuthenticator &authData)
{
  ui.userNameEdit->setText( authData.user() );
  ui.passwordEdit->setText( authData.password() );
}

void Settings::switchToPublic() {
  if ( !ui.radioPublic->isChecked() ) {
    ui.radioPublic->setChecked( true );

#if defined Q_WS_X11 || defined Q_WS_MAC
    QSettings settings( "ayoy", "qTwitter" );
#endif
#if defined Q_WS_WIN
    QSettings settings( QSettings::IniFormat, QSettings::UserScope, "ayoy", "qTwitter" );
#endif
    settings.setValue( "General/timeline", ui.radioFriends->isChecked() );
  }
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

QDir Settings::directoryOf(const QString &subdir)
{
  QDir dir(QApplication::applicationDirPath());

#if defined Q_WS_WIN
  if (dir.dirName().toLower() == "debug" || dir.dirName().toLower() == "release")
    dir.cdUp();
#elif defined Q_WS_MAC
  if (dir.dirName() == "MacOS") {
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
  }
#endif
  dir.cd(subdir);
  return dir;
}

void Settings::switchLanguage( int index )
{
  QString locale = ui.languageCombo->itemData( index ).toString();
  QString qmPath( ":/translations" );
  qDebug() << "switching language to" << locale << "from" << qmPath;
  translator.load( "qtwitter_" + locale, qmPath);
  retranslateUi();
  adjustSize();
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

void Settings::retranslateUi()
{
  ui.label->setText( tr("Refresh every") );
  ui.label_2->setText( tr("minutes") );
  ui.label_3->setText( tr("Language") );
  ui.userNameLabel->setText( tr( "Username" ) );
  ui.passwordLabel->setText( tr( "Password" ) );
  ui.tabs->setTabText( 0, tr( "General " ) );
  ui.tabs->setTabText( 1, tr( "Network " ) );
  ui.proxyBox->setText( tr( "Use HTTP &proxy" ) );
  ui.hostLabel->setText( tr( "Host:" ) );
  ui.portLabel->setText( tr( "Port:" ) );
  ui.network->setWindowTitle( tr( "Network" ) );
  ui.general->setWindowTitle( tr( "General" ) );
  ui.downloadBox->setTitle( tr( "Download" ) );
  ui.radioFriends->setText( tr( "friends timeline" ) );
  ui.radioPublic->setText( tr( "public timeline" ) );
  ui.buttonBox->button( QDialogButtonBox::Apply )->setText( tr( "Apply" ) );
  ui.buttonBox->button( QDialogButtonBox::Cancel )->setText( tr( "Cancel" ) );
  ui.buttonBox->button( QDialogButtonBox::Ok )->setText( tr( "OK" ) );
}
