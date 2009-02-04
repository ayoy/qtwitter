#ifndef LOOPEDSIGNAL_H
#define LOOPEDSIGNAL_H

#include <QThread>

class LoopedSignal : public QThread
{
  Q_OBJECT

  int period;

protected:
  virtual void run();

public:
  LoopedSignal( QObject *parent = 0 );
  void setPeriod ( int );

signals:
  void ping();  
};

#endif // LOOPEDSIGNAL_H
