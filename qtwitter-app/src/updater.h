#ifndef UPDATER_H
#define UPDATER_H

#include <QNetworkAccessManager>

class QNetworkReply;

class Updater : public QNetworkAccessManager
{
  Q_OBJECT
public:
  Updater( QObject *parent = 0 );
  virtual ~Updater();

public slots:
  void checkForUpdate();

signals:
  void updateChecked( bool available, const QString &version );

private slots:
  void readReply( QNetworkReply *reply );
};

#endif // UPDATER_H
