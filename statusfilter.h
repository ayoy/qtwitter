#ifndef STATUSFILTER_H
#define STATUSFILTER_H

class StatusFilter : public QObject
{
public:
  StatusFilter( QObject *parent = 0 ) : QObject( parent ) {}
  
protected:
  bool eventFilter( QObject *dist, QEvent *event )
  {
    if ( event->type() == QEvent::KeyPress )
    {
      QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
      if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
        return true;
    }
    return QObject::eventFilter(dist, event);
  }
};

#endif //STATUSFILTER_H