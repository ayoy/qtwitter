#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "ui_authdialog.h"
#include "entry.h"
#include "xmlparser.h"
                        
#include <QMessageBox>
#include <QtNetwork>

class HttpConnection : public QWidget {

  Q_OBJECT

public:
  HttpConnection();
  void get( const QString &path );
  
public slots:
  void httpRequestFinished( int requestId, bool error );
  void readResponseHeader( const QHttpResponseHeader &responseHeader );
  void updateDataReadProgress( int bytesRead, int totalBytes );
  void slotAuthenticationRequired( const QString &, quint16, QAuthenticator * );
  void forwardDataParsed(const QString&);
  void forwardNewEntry(const Entry&);

signals:
  void dataParsed( const QString& );
  void newEntry( const Entry& );
  void imageDownloaded( const QImage& );

private:
  QHttp *http;
  QUrl url;
  QByteArray *bytearray;
  QTextStream *textstream;
  QBuffer *buffer; 
  QFile *file;
  QImage userImage;
  QNetworkProxy proxy;
  XmlParser parser;
  bool httpRequestAborted;
  int httpGetId;
};

#endif //HTTPCONNECTION_H
