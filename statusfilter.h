#ifndef STATUSFILTER_H
#define STATUSFILTER_H
#include <QObject>
#include <QList>
#include <QEvent>
#include <qevent.h>

class StatusFilter : public QObject
{
  Q_OBJECT

public:
  StatusFilter( QObject *parent = 0 ) : QObject( parent ) 
  {
    keyList << Qt::Key_Escape << Qt::Key_Tab << Qt::Key_Backspace << Qt::Key_Insert << Qt::Key_Delete << Qt::Key_Left << Qt::Key_Right << Qt::Key_Shift;
  }

signals:
  void enterPressed( QKeyEvent *key );

protected:
  bool eventFilter( QObject *dist, QEvent *event )
  {
    if ( event->type() == QEvent::KeyPress )
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
      if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
      {
        emit enterPressed(keyEvent);
        return true;
      }
    }
    return QObject::eventFilter(dist, event);
  }
  
private:
  QList<int> keyList;
};

#endif //STATUSFILTER_H