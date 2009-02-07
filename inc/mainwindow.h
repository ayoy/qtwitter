#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settings.h"
#include "loopedsignal.h"
#include "ui_mainwindow.h"
#include "core.h"

#include <QSystemTrayIcon>

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

signals:
  void settingsDialogRequested();
  void get();
  void post( const QByteArray& );
  void openBrowser();

private:
  void resizeEvent( QResizeEvent* );
  void unlock();
  QSystemTrayIcon *icon;
  QMenu *menu;
  LoopedSignal *repeat;
  QStandardItemModel model;
  Settings *settingsDialog;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
