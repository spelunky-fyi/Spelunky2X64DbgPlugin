#pragma once

#include <QFocusEvent>
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
        void focused(bool hasFocus);
        void mouseRelease(Qt::MouseButton button);

      protected:
        virtual void focusInEvent(QFocusEvent* e)
        {
            QLineEdit::focusInEvent(e);
            emit focused(true);
        }
        virtual void focusOutEvent(QFocusEvent* e)
        {
            QLineEdit::focusInEvent(e);
            emit focused(false);
        }
        virtual void mouseReleaseEvent(QMouseEvent* e)
        {
            QLineEdit::mouseReleaseEvent(e);
            emit mouseRelease(e->button());
        }
    };
} // namespace S2Plugin
