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

void StatusEdit::initialize() {
  setText( tr( "What are you doing?" ) );
  statusClean = true;
}
