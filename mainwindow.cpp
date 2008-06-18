#include "mainwindow.h"
#include "statusfilter.h"

//#include <QtDebug>

MainWindow::MainWindow() : QDialog()
{
  ui.setupUi( this );
  StatusFilter *filter = new StatusFilter();
  ui.statusEdit->installEventFilter( filter );
  connect( ui.statusEdit, SIGNAL( textChanged() ), this, SLOT( changeLabel() ) );
  //connect( this, SIGNAL( statusChanged() ), this, SLOT( changeLabel() ) );
}

/*void MainWindow::addItem()
{
  EditDialog dlg( this );

  if( dlg.exec() == QDialog::Accepted )
    ui.list->addItem( dlg.name() + " -- " + dlg.number() );
}*/

void MainWindow::changeLabel()
{
  static int formerLength = -1;
  //qDebug() << formerLength;
  if (formerLength != ui.statusEdit->toPlainText().length() ) 
  {
    formerLength = ui.statusEdit->toPlainText().length();
    ui.statusEdit->setPlainText( ui.statusEdit->toPlainText().left( STATUS_MAX_LEN ) );
    ui.statusEdit->moveCursor( QTextCursor::End );
  }
  ui.countdownLabel->setText( QString::number( STATUS_MAX_LEN - ui.statusEdit->toPlainText().length() ) );
}
