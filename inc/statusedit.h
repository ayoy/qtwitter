#ifndef STATUSEDIT_H
#define STATUSEDIT_H

#include <QLineEdit>

class StatusEdit : public QLineEdit
{
  Q_OBJECT
public:
  StatusEdit( QWidget * parent );
  void focusInEvent( QFocusEvent * event );
  void focusOutEvent( QFocusEvent * event );
  void initialize();
  inline bool isStatusClean() { return statusClean; }

public slots:
  void cancelEditing();

private:
  bool statusClean; 
};

#endif //STATUSEDIT_H
