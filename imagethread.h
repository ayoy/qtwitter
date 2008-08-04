#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QStandardItemModel>

#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"


class ImageThread : public QThread {
  Q_OBJECT

public:
  ImageThread();
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
  XmlDownload http;
  XmlDownload upload;
  bool xmlBeingProcessed;
  QMutex mutex;
  ImageDownload imageDownload;
  QMap<QString, QImage> imagesHash;
  QStandardItemModel model;
  QList<Entry> entries;
  QImage userImage;
  Entry userEntry;

};


#endif //IMAGETHREAD_H