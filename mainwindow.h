#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include "ui_mainwindow.h"

#define STATUS_MAX_LEN 140

class MainWindow : public QDialog
{
  Q_OBJECT

public:
  MainWindow();

private slots:
  void changeLabel();

private:
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
