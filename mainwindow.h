#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <qevent.h>
#include <QStandardItemModel>

#include "statusedit.h"
#include "ui_mainwindow.h"
#include "entry.h"
#include "httpconnection.h"

#define STATUS_MAX_LEN 140
#define ICON_SIZE 48
#define SCROLLBAR_MARGIN 20
#define ITEM_SPACING 10

/*class ImageInfo {
public:
  ImageInfo() : QImage(), Entry() {};
  ImageInfo( const Entry &other ) : QImage(), Entry( other ) {};
//  ImageInfo( const QString &url, const Entry &other ) : QImage( url ), Entry( other ) {};
  bool isImageDefined( const QString& url ) { return ( details.userImage() == url ); }

private:
  QImage image;
  Entry details;

};*/


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
  void saveImage( const QImage &image );
  void popupError( const QString &message );
  
  void resizeEvent( QResizeEvent *event );
  
private:  
  HttpConnection http;
  HttpConnection imageDownload; 
  QMap<QString, QImage> imagesHash;
  QStandardItemModel model;
  QImage userImage;
  Entry userEntry;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
