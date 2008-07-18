#include "statusedit.h"

StatusEdit::StatusEdit( QWidget * parent = 0 ) :
  QLineEdit( parent ),
  statusClean( true )
  {}

void StatusEdit::focusInEvent( QFocusEvent * event )
{
  if ( statusClean == true )
  {
    setText( "" );
    statusClean = false;
  }
  QLineEdit::focusInEvent(event);
}

bool StatusEdit::isStatusClean() const
{
  return statusClean;
}

void StatusEdit::setStatusClean( const bool &set )
{
  statusClean = set;
}
