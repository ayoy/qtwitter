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
  void sendStatus( QKeyEvent *key );
  void resetStatus();
 
  void updateText( const QString& text );
  void addEntry( const Entry &entry );
  void saveImage( const QString &imageUrl, const QImage &image );
  void downloadImages();
  void popupError( const QString &message );
  
  void resizeEvent( QResizeEvent *event );
  
private:  
  void display();
  XmlDownload http;
  ImageDownload imageDownload; 
  QMap<QString, QImage> imagesHash;
  QStandardItemModel model;
  QList<Entry> entries;
  QImage userImage;
  Entry userEntry;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
