#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>

#include "statusedit.h"
#include "statusfilter.h"
#include "ui_mainwindow.h"
#include "ui_settings.h"
#include "core.h"

#define STATUS_MAX_LEN 140
#define ICON_SIZE 48
#define SCROLLBAR_MARGIN 20
#define ITEM_SPACING 10

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();
  void resizeEvent( QResizeEvent *event );
  void checkAlign( int width );
  
public slots:
  void changeLabel();
  void sendStatus();
  void resetStatus();
 
  void updateTweets();
  void openSettings();
  void popupError( const QString &message );
  

  void display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash );
  
private:
  void unlockState();
  Core threadingEngine;
  QStandardItemModel model;
  QFontMetrics *fm;
  QNetworkProxy proxy;
  StatusFilter *filter;
  Ui::MainWindow ui;
  Ui::Settings ui_s;
};

#endif //MAINWINDOW_H
