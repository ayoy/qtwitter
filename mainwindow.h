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

public slots:
  void changeLabel();
  void sendStatus(QKeyEvent *key);
  void resetStatus();

  void httpRequestFinished(int requestId, bool error);
  void readResponseHeader(const QHttpResponseHeader &responseHeader);
  void updateDataReadProgress(int bytesRead, int totalBytes);
  void slotAuthenticationRequired(const QString &, quint16, QAuthenticator *);

private:
  QHttp *http;
  QFile *file;
  QNetworkProxy proxy;
  int statusFormerLength;
  bool httpRequestAborted;
  int httpGetId;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
