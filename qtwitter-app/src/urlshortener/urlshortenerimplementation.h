#ifndef URLSHORTENERIMPLEMENTATION_H
#define URLSHORTENERIMPLEMENTATION_H

#include <QObject>
#include "urlshortener.h"

class UrlShortenerImplementation : public QObject
{
  Q_OBJECT

public:
  UrlShortenerImplementation( QObject *parent = 0 );
  virtual ~UrlShortenerImplementation();
  virtual UrlShortener::Shortener shorteningService() = 0;
  virtual void shorten( const QString &url ) = 0;

protected:
  QNetworkAccessManager *connection;
  int replyStatus( QNetworkReply *reply ) const;

signals:
  void shortened( const QString &url );
  void errorMessage( const QString &message );

protected slots:
  virtual void replyFinished( QNetworkReply* ) = 0;
};

class IsgdShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  IsgdShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_ISGD; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

class TrimShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  TrimShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_TRIM; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

class MetamarkShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  MetamarkShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_METAMARK; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

class TinyurlShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  TinyurlShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_TINYURL; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

class TinyarrowsShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  TinyarrowsShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_TINYARROWS; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

class UnuShortener : public UrlShortenerImplementation
{
  Q_OBJECT

public:
  UnuShortener( QObject *parent = 0 );
  UrlShortener::Shortener shorteningService() { return UrlShortener::SHORTENER_UNU; }
  void shorten( const QString &url );
protected slots:
  virtual void replyFinished( QNetworkReply* );
};

#endif // URLSHORTENERIMPLEMENTATION_H
