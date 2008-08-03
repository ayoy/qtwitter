#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <qevent.h>
#include <QStandardItemModel>

#include "statusedit.h"
#include "ui_mainwindow.h"
#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"
#include "imagethread.h"

#define STATUS_MAX_LEN 140
#define ICON_SIZE 48
#define SCROLLBAR_MARGIN 20
#define ITEM_SPACING 10

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow();
  
public slots:
  void changeLabel();
  void sendStatus();
  void resetStatus();
 
  void updateTweets();
  void popupError( const QString &message );
  
  void resizeEvent( QResizeEvent *event );
  void display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash );
  
private:  
  ImageThread imageSaver;
  QStandardItemModel model;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
