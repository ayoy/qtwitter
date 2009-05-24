#ifndef DOMPARSER_H
#define DOMPARSER_H

#include <QObject>
#include <QDomDocument>
#include <QByteArray>
#include "entry.h"
#include "twitterapi.h"


class DomParser : public QObject
{
  Q_OBJECT

public:
    DomParser(TwitterAPI::SocialNetwork network, const QString &login, QObject *parent);
    void parse();
    void setContent(const QByteArray &data, int role);

protected:
    void parseUserInfo();

signals:
  //void newEntry( TwitterAPI::SocialNetwork network, const QString &login, Entry entry );

private:
    QDomDocument reply;
    TwitterAPI::SocialNetwork network;
    QString login;
    Entry entry;
    int role;
};

#endif // DOMPARSER_H
