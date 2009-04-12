#ifndef URLSHORTEN_H
#define URLSHORTEN_H

#include <QObject>

class QNetworkReply;
class QNetworkAccessManager;

class UrlShorten : public QObject
{
    Q_OBJECT

public:
    UrlShorten( QObject *parent = 0 );
    ~UrlShorten();
    void shorten( const QString &url );

private:
    QNetworkAccessManager *manager;

signals:
    void shortened( const QString &url );
    void errorMessage( const QString &message );

private slots:
    void replyFinished( QNetworkReply* );
};

#endif // URLSHORTEN_H
