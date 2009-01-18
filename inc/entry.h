#ifndef ENTRY_H
#define ENTRY_H

#include <QRegExp>
#include <QDebug>

class Entry {

private:
  QString userName;
  QString userImage;
  QString userText;
  //QRegExp ahref;
  
public:
  Entry() :
    userName( "" ),
    userImage( "" ),
    userText( "" )
    //ahref( "(http://[^ ]+) ?", Qt::CaseInsensitive )
    {}
  Entry(const QString &name, const QString &image, const QString &text) :
    userName( name ),
    userImage( image ),
    userText( text )
    //ahref( "(http://[^ ]+) ?", Qt::CaseInsensitive )
    {}
  Entry(const Entry &right) :
    userName( right.userName ),
    userImage( right.userImage ),
    userText( right.userText )
    //ahref( "(http://[^ ]+) ?", Qt::CaseInsensitive )
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
  void setText( const QString& newText ) {
    userText = newText;
    qDebug() << "PRZED:\n" << userText;
    QRegExp ahref( "(http://[^ ]+) ?", Qt::CaseInsensitive );
    //ahref.setPattern( "(http://[^ ]+) ?" );
    userText.replace( ahref, "<a href=\\1>\\1</a>" );
    qDebug() << "PO 1:\n" << userText;
    ahref.setPattern( "(<a href=[^ ]+)/+&gt" );
    ahref.setMinimal( true );
    userText.replace( ahref, "\\1>" );
    qDebug() << "PO 2:\n" << userText;
  }
};

#endif //ENTRY_H
