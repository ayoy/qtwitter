#include "entry.h"

#include <QRegExp>
#include <QDebug>
#include <QList>
#include <QMetaType>

Entry::Entry() :
    userName( "" ),
    userImage( "" ),
    userText( "" )
{
}

Entry::Entry(const QString &name, const QString &image, const QString &text) :
    userName( name ),
    userImage( image ),
    userText( text )
{
}

Entry::Entry(const Entry &right) :
    userName( right.userName ),
    userImage( right.userImage ),
    userText( right.userText )
{
}

bool Entry::checkContents() {
  if ( userName.compare( "" ) &&
       userImage.compare( "" ) &&
       userText.compare( "" ) ) {
    return true;
  }
  return false;
}

Entry& Entry::operator=( const Entry &right ) {
  userName = right.userName;
  userImage = right.userImage;
  userText = right.userText;
  return *this;
}

QString Entry::name() const {
  return userName;
}

QString Entry::image() const {
  return userImage;
}

QString Entry::text() const {
  return userText;
}

void Entry::setName( const QString& newName ) {
  userName = newName;
}

void Entry::setImage( const QString& newImage ) {
  userImage = newImage;
}

void Entry::setText( const QString& newText ) {
  userText = newText;
  QRegExp ahref( "(http://[^ ]+)( ?)", Qt::CaseInsensitive );
  userText.replace( ahref, "<a href=\\1>\\1</a>\\2" );
  userText.replace( QRegExp( "(^| )@([^ ]+)( ?)" ), " <a href=http://twitter.com/\\2>@\\2</a>\\3" );
  ahref.setPattern( "(<a href=[^ ]+)/>" );
  ahref.setMinimal( true );
  userText.replace( ahref, "\\1>" );
}
