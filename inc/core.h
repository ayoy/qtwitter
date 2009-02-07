#ifndef CORE_H
#define CORE_H

#include <QStandardItemModel>
#include <QAuthenticator>

#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"

typedef QList<Entry> ListOfEntries;
typedef QMap<QString, QImage> MapStringImage;

class Core : public QThread {
  Q_OBJECT

public:
  Core( QObject *parent = 0 );
  virtual ~Core();
  bool downloadsPublicTimeline();

protected:
  void run();

public slots:
  void get();
  void post( const QByteArray &status );

  void authDataDialog();
  void setAuthData( const QString &name, const QString &password );
  void addEntry( const Entry &entry, int type );
  void downloadImages();
  void error( const QString &message );
  void storeCookie( const QStringList );
  void setDownloadPublicTimeline( bool );

private slots:
  void destroyXmlConnection();

signals:
  void readyToDisplay( const ListOfEntries &entries, const MapStringImage &imagesHash );
  void errorMessage( const QString &message );
  void authDataSet( const QAuthenticator& );
  void updateNeeded();
  void xmlConnectionIdle();

private:
  bool xmlBeingProcessed;
  bool downloadPublicTimeline;
  XmlDownload *xmlGet;
  XmlDownload *xmlPost;
  ImageDownload *imageDownload;
  MapStringImage imagesHash;
  ListOfEntries entries;
  QAuthenticator authData;
  QStringList cookie;
  QStandardItemModel model;
};


#endif //CORE_H
