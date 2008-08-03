#ifndef IMAGETHREAD_H
#define IMAGETHREAD_H

#include <QStandardItemModel>

#include "statusedit.h"
#include "entry.h"
#include "xmldownload.h"
#include "imagedownload.h"


class ImageThread : public QThread {
  Q_OBJECT

public:
  ImageThread();

protected:
  void run();

public slots:

  void addEntry( const Entry &entry );
  void saveImage( const QString &imageUrl, const QImage &image );
  void downloadImages();
    
signals:
  void readyToDisplay( const QList<Entry> &entries, const QMap<QString, QImage> &imagesHash );
  
public:
  XmlDownload http;
  
private:
  QMutex mutex;
  ImageDownload imageDownload;
  QMap<QString, QImage> imagesHash;
  QStandardItemModel model;
  QList<Entry> entries;
  QImage userImage;
  Entry userEntry;

};


#endif //IMAGETHREAD_H