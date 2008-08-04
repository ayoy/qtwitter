#ifndef CORE_H
#define CORE_H

#include <QStandardItemModel>

#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"


class Core : public QThread {
  Q_OBJECT

public:
  Core();
  void get( const QString &path );
  void post( const QString &path, const QByteArray &status );

protected:
  void run();

public slots:

  void addEntry( const Entry &entry, int type );
  void saveImage( const QString &imageUrl, const QImage &image );
  void downloadImages();
  void error( const QString &message );
    
signals:
  void readyToDisplay( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash );
  void errorMessage( const QString &message );
  
private:
  bool xmlBeingProcessed;
  XmlDownload xmlGet;
  XmlDownload xmlPost;
  ImageDownload imageDownload;
  QMap<QString, QImage> imagesHash;
  QList<Entry> entries;
  QStandardItemModel model;
};


#endif //CORE_H