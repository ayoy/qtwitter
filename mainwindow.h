#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QtNetwork>
#include <qevent.h>
#include "statusedit.h"
#include "ui_mainwindow.h"

#define STATUS_MAX_LEN 140

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  MainWindow();

private slots:
  void changeLabel();
  void sendStatus(QKeyEvent *key);
  void resetStatus();

private:
  //QHttp *http;
  int statusFormerLength;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
