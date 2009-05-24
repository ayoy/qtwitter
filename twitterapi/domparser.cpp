#include <QDomDocument>
#include <QDebug>
#include "domparser.h"
#include "twitterapi.h"
#include "entry.h"


DomParser::DomParser(TwitterAPI::SocialNetwork network, const QString &login, QObject *parent) :
    QObject( parent ), entry()
{
  this->network = network;
  this->login = login;
}

void DomParser::setContent(const QByteArray &data, int role)
{
  reply.setContent(data, false);
  this->role = role;
}

void DomParser::parse()
{
  switch (role) {
    case TwitterAPI::ROLE_USERINFO:
      parseUserInfo();
      break;
    default:
      qDebug() << "Nothing to parse";
  }
}

void DomParser::parseUserInfo()
{
  QDomElement elem = reply.documentElement();
  QDomNode n = elem.firstChild();
  while ( !n.isNull() ) {
    QDomElement e = n.toElement();
    if ( !e.isNull() ) {
      qDebug() << qPrintable( e.tagName() );
      if(e.tagName() == "id")
        entry.id = e.text().toInt();
      else if(e.tagName() == "screen_name")
        entry.name = e.text();
    }
    n = n.nextSibling();
  }
//  emit completed(true, url, userId);
//  emit newEntry(network, login, entry);
}

/* void TwitPicEngine::parseReply(QByteArray &reply)
{
  QString url;
  bool userId = false;
  QDomDocument doc;

  doc.setContent(reply, false);
  QDomElement docElem = doc.documentElement();

  if ( docElem.hasAttribute("stat") && (docElem.attribute("stat") == "fail") ) {

    //failures have only att. "stat"
    QDomElement error = docElem.firstChild().toElement();
    qDebug() << error.tagName();
    if (!error.isNull()) {
      int errCode = error.attribute("code").toInt();
      QString errMsg;

      switch (errCode) {
      case ErrInvalidLogin:
        errMsg = tr( "Invalid twitter username or password");
        break;
      case ErrImageNotFound:
        errMsg = tr( "Image not found" );
        break;
      case ErrInvalidType:
        errMsg = tr( "Invalid image type" );
        break;
      case ErrOversized:
        errMsg = tr( "Image larger than 4MB" );
        break;
      default:
        errMsg = tr( "We're sorry, but due to some mysterious error your image failed to upload" );
      }
      emit completed(false, "\n" + errMsg, false);
      return;
    }

    //status ok:
    QDomNode n = docElem.firstChild();
    while ( !n.isNull() ) {
      QDomElement e = n.toElement();
      if ( !e.isNull() ) {
        qDebug() << qPrintable( e.tagName() );
        if(e.tagName() == "userid")
          userId = true;
        else if(e.tagName() == "mediaurl")
          url = e.text();
      }
      n = n.nextSibling();
    }
    emit completed(true, url, userId);
  }
}
*/
