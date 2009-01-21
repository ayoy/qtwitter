#ifndef ENTRY_H
#define ENTRY_H

#include <QString>

class Entry {

private:
  QString userName;
  QString userImage;
  QString userText;
  
public:
  Entry();
  Entry(const QString &name, const QString &image, const QString &text);
  Entry(const Entry &right);

  bool checkContents();
  Entry& operator=( const Entry &right );
      
  QString name() const;
  QString image() const;
  QString text() const;
  
  void setName( const QString& newName );
  void setImage( const QString& newImage );
  void setText( const QString& newText );
};

#endif //ENTRY_H
