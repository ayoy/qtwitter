#ifndef ENTRY_H
#define ENTRY_H

#include <QString>
#include <QUrl>

class Entry {

public:
  Entry() :
    userName( "" ),
    userAvatar( "" ),
    userStatus( "" )
    {}
  Entry(const QString &name, const QUrl &avatar, const QString &status) :
    userName( name ),
    userAvatar( avatar ),
    userStatus( status )
    {}
    
  const QString& name() const { return userName; }
  const QUrl& avatar() const { return userAvatar; }
  const QString& status() const { return userStatus; }
  
  void setName( const QString& newName ) { userName = newName; }
  void setAvatar( const QUrl& newAvatar ) { userAvatar = newAvatar; }
  void setStatus( const QString& newStatus ) { userStatus = newStatus; }
  

private:
  QString userName;
  QUrl userAvatar;
  QString userStatus;
  
};

#endif