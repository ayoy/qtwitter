#include "settings.h"

Settings::Settings( QTranslator &_translator, QWidget *parent ) : QDialog( parent ), translator( _translator ) {
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
  connect( ui.languageCombo, SIGNAL( currentIndexChanged( const QString& )), this, SLOT( changeLanguage( const QString& ) ) );
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
  QDialog::accept();
}

void Settings::changeLanguage( const QString& /*lang*/ ) {
  qDebug() << "CHANGE LANGUAGE KPLZ";
  translator.load( "qtwitter_ca" );
  qApp->installTranslator( &translator );
}
