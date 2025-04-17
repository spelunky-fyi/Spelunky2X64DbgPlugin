#pragma once

#include <QLineEdit>
#include <QMouseEvent>

namespace S2Plugin
{
    class LineEditEx : public QLineEdit
    {
        Q_OBJECT
      public:
        LineEditEx(QWidget* parent = 0) : QLineEdit(parent){};

      signals:
        void focussed(bool hasFocus);
        void mouseRelease(Qt::MouseButton button);

      protected:
        virtual void focusInEvent(QFocusEvent* e)
        {
            QLineEdit::focusInEvent(e);
            emit(focussed(true));
        }
        virtual void focusOutEvent(QFocusEvent* e)
        {
            QLineEdit::focusInEvent(e);
            emit(focussed(false));
        }
        virtual void mouseReleaseEvent(QMouseEvent* e)
        {
            QLineEdit::mouseReleaseEvent(e);
            emit(mouseRelease(e->button()));
        }
    };
} // namespace S2Plugin
