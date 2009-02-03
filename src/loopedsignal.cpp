#include "loopedsignal.h"

LoopedSignal::LoopedSignal( int _period, QObject *parent ) : QThread( parent ), period ( _period ) {}

void LoopedSignal::run()
{
  forever {
    emit ping();
    sleep( period );
  }
}

void LoopedSignal::setPeriod( int _period )
{
  if ( period != _period ) {
    terminate();
    period = _period;
    start();
  }
}
