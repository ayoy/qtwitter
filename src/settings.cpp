#include "settings.h"

#include <QtXml>

Settings::Settings( QWidget *parent ) : QDialog( parent ) {
/*  QStringList env( QProcess::systemEnvironment() );
  QString confFileName = env.join( ";" );

#if defined Q_WS_X11 || defined Q_WS_MAC
  QRegExp homeRX( ".*;HOME=([^;]+);.*", Qt::CaseSensitive );
#endif

  if ( homeRX.exactMatch( confFileName ) ) {
    qDebug() << "matches";
    confFileName = homeRX.cap( 1 );
  }

#ifdef Q_WS_X11
  confFileName.append( "/.qtwitter" );
#endif
#ifdef Q_WS_MAC
  confFileName.append( "/.qtwitter" );
#endif
#ifdef Q_WS_WIN
  confFileName.append( "/qtwitter.cfg" );
#endif

  qDebug() << confFileName;
  configFile.setFileName( confFileName );
*/
  QCoreApplication::setOrganizationDomain( "ayoy.net" );


  ui.setupUi( this );
  QIntValidator *portValidator= new QIntValidator( 1, 65535, this );
  ui.portEdit->setValidator( portValidator );
  proxy.setType( QNetworkProxy::NoProxy );
  if ( proxy.type() != QNetworkProxy::NoProxy ) {
    ui.proxyBox->setChecked( true );
    ui.hostEdit->setEnabled( true );
    ui.portEdit->setEnabled( true );
  }
  ui.hostEdit->setText( proxy.hostName() );
  if ( proxy.port() ) {
    ui.portEdit->setText( QString::number( proxy.port() ) );
  }
  //connect( ui.languageCombo, SIGNAL( currentIndexChanged( const QString& )), this, SLOT( changeLanguage( const QString& ) ) );
}

Settings::~Settings() {}

void Settings::accept() {
  if ( ui.proxyBox->isChecked() ) {
    proxy.setType( QNetworkProxy::HttpProxy );
    proxy.setHostName( ui.hostEdit->text() );
    proxy.setPort( ui.portEdit->text().toInt() );
  } else {
    proxy.setType( QNetworkProxy::NoProxy );
  }
  QNetworkProxy::setApplicationProxy( proxy );
  saveConfig();
  QDialog::accept();
}

void Settings::changeLanguage( const QString& /*lang*/ ) {
  //qDebug() << "CHANGE LANGUAGE KPLZ";
  //translator.load( "qtwitter_ca" );
  //qApp->installTranslator( &translator );
}

bool createConfigFile() {
  return true;
}

bool Settings::loadConfig() {
  /*QDomDocument config( "qtwitter" );
  bool openConfig = false;
  if ( configFile.exists() ) {
    qDebug() << "conf file exists";
    openConfig = configFile.open( QIODevice::ReadWrite | QIODevice::Text );
  } else {
    qDebug() << "conf file not found, creating one";
    openConfig = configFile.open( QIODevice::WriteOnly | QIODevice::Text );
  }
  if ( openConfig ) {
    QTextStream out( &configFile );
    config.setContent( &configFile );

    QDomElement docElem = config.documentElement();
    QDomElement elem = config.createElement( "img" );
    elem.setAttribute( "src", "myimage.png" );
    config.appendChild( elem );
    qDebug() << config.toString().toAscii();
    out << config.toString();
    configFile.close();
  }*/

  QSettings settings( "ayoy", "qTwitter" );

  settings.beginGroup( "General" );
  ui.refreshCombo->setCurrentIndex( settings.value( "refresh" ).toInt() );
  ui.languageCombo->setCurrentIndex( settings.value( "languge", 1 ).toInt() );
  settings.endGroup();
  settings.beginGroup( "Network" );
  settings.beginGroup( "Proxy" );
  ui.proxyBox->setCheckState( (Qt::CheckState)settings.value( "enabled" ).toInt() );
  ui.hostEdit->setText( settings.value( "host" ).toString() );
  ui.portEdit->setText( settings.value( "port" ).toString() );
  settings.endGroup();
  settings.endGroup();

  return true;
}

bool Settings::saveConfig() {
  /*QDomDocument config( "qtwitter" );
  if ( configFile.exists() ) {
    qDebug() << "conf file (" << configFile.fileName() << ") exists";
  } else {
    qDebug() << "conf file (" << configFile.fileName() << ") not found, creating one";
  }
  if ( configFile.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
    QTextStream out( &configFile );
    config.setContent( &configFile );

    QDomElement domSettings = config.createElement( "settings" );
    QDomElement domGeneral = config.createElement( "general" );
      QDomElement domRefresh = config.createElement( "refresh" );
        QDomText domRefreshVal = config.createTextNode( ui.refreshCombo->currentText() );
    QDomElement domNetwork = config.createElement( "network" );
      QDomElement domProxy = config.createElement( "proxy" );
        QDomElement domEnabled = config.createElement( "enabled" );
          QDomText domEnabledVal = config.createTextNode( stateForXML( ui.proxyBox ) );
        QDomElement domHost = config.createElement( "host" );
          QDomText domHostVal = config.createTextNode( ui.hostEdit->text() );
        QDomElement domPort = config.createElement( "port" );
          QDomText domPortVal = config.createTextNode( ui.portEdit->text() );

    config.appendChild( domSettings );
    domSettings.appendChild( domGeneral );
    domGeneral.appendChild( domRefresh );
    domRefresh.appendChild( domRefreshVal );
    domSettings.appendChild( domNetwork );
    domNetwork.appendChild( domProxy );
    domProxy.appendChild( domEnabled );
    domEnabled.appendChild( domEnabledVal );
    domProxy.appendChild( domHost );
    domHost.appendChild( domHostVal );
    domProxy.appendChild( domPort );
    domPort.appendChild( domPortVal );

    //qDebug() << config.toString().toAscii();
    //out << config.toString();
    config.save( out, 2 );
    configFile.close();
  }*/
  QSettings settings( "ayoy", "qTwitter" );

  settings.beginGroup( "General" );
  settings.setValue( "refresh", ui.refreshCombo->currentIndex() );
  settings.setValue( "language", ui.languageCombo->currentIndex() );
  settings.endGroup();
  settings.beginGroup( "Network" );
  settings.beginGroup( "Proxy" );
  settings.setValue( "enabled", ui.proxyBox->checkState() );
  settings.setValue( "host", ui.hostEdit->text() );
  settings.setValue( "port", ui.portEdit->text() );
  settings.endGroup();
  settings.endGroup();

  return true;
}

QString Settings::stateForXML ( QCheckBox *checkBox ) {
  if ( checkBox->checkState() == Qt::Checked ) {
      return "1";
  }
  return "0";
}
