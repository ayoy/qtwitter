#include "tweet.h"
#include "ui_tweet.h"

#include <QDebug>
#include <QScrollBar>

Tweet::Tweet(QWidget *parent) :
  QWidget(parent),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi(this);
}

Tweet::Tweet( const QString &name, const QString &status, const QImage &icon, QWidget *parent ) :
  QWidget(parent),
  m_ui(new Ui::Tweet)
{
  m_ui->setupUi( this );
  //connect( m_ui->userStatus, SIGNAL( textChanged() ), this, SLOT( adjustSize() ) );
  m_ui->userName->setText( name );
  m_ui->userStatus->setText( status );
  m_ui->userIcon->setPixmap( QPixmap::fromImage( icon ) );
  adjustSize();
}

Tweet::~Tweet()
{
  delete m_ui;
}

void Tweet::resize( const QSize &s ) {
  m_ui->frame->resize( s );
  QWidget::resize( s );
}

void Tweet::resize( int w, int h ) {
  size().setWidth( w );
  //size().setHeight( h );
  m_ui->frame->size().setWidth( w );
  //adjustSize();
  qDebug() << "setting width to" << m_ui->frame->size().width();
  qDebug() << "setting height to" << m_ui->frame->size().height();
  //QWidget::resize( w, h );
}

void Tweet::adjustSize() {
  m_ui->userStatus->document()->setTextWidth( m_ui->userStatus->width() );
  m_ui->userStatus->resize( m_ui->userStatus->size().width(), (int)m_ui->userStatus->document()->size().height() );
  m_ui->frame->resize( m_ui->frame->width(), ( 68 > m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11) ? 68 : m_ui->userStatus->geometry().y() + m_ui->userStatus->size().height() + 11 );
  resize( m_ui->frame->size() );
}

void Tweet::changeEvent(QEvent *e) {
  switch (e->type()) {
  case QEvent::LanguageChange:
    m_ui->retranslateUi(this);
    break;
  default:
    break;
  }
}
