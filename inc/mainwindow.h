#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMessageBox>

#include "settings.h"
#include "statusedit.h"
#include "statusfilter.h"
#include "updateslist.h"
#include "ui_mainwindow.h"
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
  
public slots:
  void changeLabel();
  void sendStatus();
  void resetStatus();
 
  void updateTweets();
  void popupError( const QString &message );
  void popupMenu();

  void display( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash );
  
private:
  void resizeEvent( QResizeEvent* );
  void unlockState();
  QMenu *menu;
  Core threadingEngine;
  QStandardItemModel model;
  QFontMetrics *fm;
  StatusFilter *filter;
  Settings *settingsDialog;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
