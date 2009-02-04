#include "loopedsignal.h"

LoopedSignal::LoopedSignal( QObject *parent ) : QThread( parent ), period( 0 ) {}

void LoopedSignal::run()
{
  if ( period == 0 )
    return;
  forever {
    emit ping();
    sleep( period );
  }
}

void LoopedSignal::setPeriod( int _period )
{
  if ( period != _period ) {
    if ( this->isRunning() )
      terminate();
    period = _period;
    start();
  }
}
