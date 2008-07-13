#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QtNetwork>
#include <qevent.h>
#include <QStandardItemModel>
#include "statusedit.h"
#include "ui_mainwindow.h"
#include "xmlparser.h"
#include "entry.h"

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
  
  void updateText( const QString& text );
  void addEntry( const Entry &entry );

private:
  QHttp *http;
  //QFile *file;
  QByteArray *bytearray;
  QTextStream *textstream;
  QStandardItemModel model;
  QBuffer *buffer; 
  QNetworkProxy proxy;
  XmlParser parser;
  int statusFormerLength;
  bool httpRequestAborted;
  int httpGetId;
  Ui::MainWindow ui;
};

#endif //MAINWINDOW_H
