#ifndef ENTRY_H
#define ENTRY_H

#include <QString>
#include <QUrl>

class Entry {

private:
  QString userName;
  QString userImage;
  QString userText;
  
public:
  Entry() :
    userName( "" ),
    userImage( "" ),
    userText( "" )
    {}
  Entry(const QString &name, const QString &image, const QString &text) :
    userName( name ),
    userImage( image ),
    userText( text )
    {}
  Entry(const Entry &right) :
    userName( right.userName ),
    userImage( right.userImage ),
    userText( right.userText )
    {}
  bool checkContents() { 
    if ( userName.compare( "" ) && 
         userImage.compare( "" ) &&
         userText.compare( "" ) ) {
      return true;
    }
    return false;
  }
  Entry& operator=( const Entry &right ) {
    userName = right.userName;
    userImage = right.userImage;
    userText = right.userText;
    return *this;
  }
      
  QString name() const { return userName; }
  QString image() const { return userImage; }
  QString text() const { return userText; }
  
  void setName( const QString& newName ) { userName = newName; }
  void setImage( const QString& newImage ) { userImage = newImage; }
  void setText( const QString& newText ) { userText = newText; }
};

#endif //ENTRY_H
