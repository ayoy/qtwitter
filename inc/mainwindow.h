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
  void retranslateUi();
  
public slots:
  void updateTweets();
  void popupError( const QString &message );
  void display( const ListOfEntries &entries, const MapStringImage &imagesHash );

private slots:
  void iconActivated( QSystemTrayIcon::ActivationReason reason );
  void changeLabel();
  void sendStatus();
  void resetStatus();
  void popupMenu();

signals:
  void settingsDialogRequested();
  void get();
  void post( const QByteArray& );
  void openBrowser();

protected:
  void closeEvent( QCloseEvent *e );

private:
  void resizeEvent( QResizeEvent* );
  void unlock();
  QMenu *menu;
  QSystemTrayIcon *trayIcon;
  LoopedSignal *repeat;
  QStandardItemModel model;
  Settings *settingsDialog;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
