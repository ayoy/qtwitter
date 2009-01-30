#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings.h"
#include "statusedit.h"
#include "statusfilter.h"
#include "loopedsignal.h"
#include "ui_mainwindow.h"
#include "core.h"

#define STATUS_MAX_LEN 140

typedef QList<Entry> ListOfEntries;
typedef QMap<QString, QImage> MapStringImage;

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

  void display( const ListOfEntries &entries, const MapStringImage &imagesHash );
  void saveConfig();
  
private:
  void resizeEvent( QResizeEvent* );
  void unlockState();
  void loadConfig();
  QMenu *menu;
  Core core;
  LoopedSignal *repeat;
  QStandardItemModel model;
  StatusFilter *filter;
  Settings *settingsDialog;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
