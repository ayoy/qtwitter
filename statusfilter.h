#ifndef STATUSFILTER_H
#define STATUSFILTER_H
#include <QObject>
#include <QEvent>
#include <qevent.h>

class StatusFilter : public QObject
{
  Q_OBJECT

public:
  StatusFilter( QObject *parent = 0 ) : QObject( parent ) 
  {
  }

signals:
  void enterPressed();

protected:
  bool eventFilter( QObject *dist, QEvent *event )
  {
    if ( event->type() == QEvent::KeyPress )
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
      if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
      {
        emit enterPressed();
        return true;
      }
    }
    return QObject::eventFilter(dist, event);
  }
};

#endif //STATUSFILTER_H
