#ifndef STATUSEDIT_H
#define STATUSEDIT_H

#include <QLineEdit>

class StatusEdit : public QLineEdit
{
  Q_OBJECT
public:
  StatusEdit( QWidget * parent );
  void focusInEvent( QFocusEvent * event );
  void initialize();
  
private:
  bool statusClean; 
};

#endif //STATUSEDIT_H
