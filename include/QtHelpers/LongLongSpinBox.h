#pragma once

#include <QAbstractSpinBox>
#include <QLineEdit>
#include <QString>
#include <QValidator>
#include <QWidget>
#include <limits>

namespace S2Plugin
{
    class LongLongSpinBox : public QAbstractSpinBox
    {
        Q_OBJECT

        using type = long long;

        type m_minimum;
        type m_maximum;
        type m_value;

      public:
        explicit LongLongSpinBox(QWidget* parent = nullptr) : QAbstractSpinBox(parent)
        {
            setRange(std::numeric_limits<type>::min(), std::numeric_limits<type>::max());
            QObject::connect(lineEdit(), &QLineEdit::textEdited, this, &LongLongSpinBox::onEditFinished);
        };
        ~LongLongSpinBox(){};

        type value() const
        {
            return m_value;
        };

        type minimum() const
        {
            return m_minimum;
        };

        void setMinimum(type min)
        {
            m_minimum = min;
        }

        type maximum() const
        {
            return m_maximum;
        };

        void setMaximum(type max)
        {
            m_maximum = max;
        }

        void setRange(type min, type max)
        {
            setMinimum(min);
            setMaximum(max);
        }

        void stepBy(int steps) override
        {
            type new_value;

            if (steps > 0 && m_value > std::numeric_limits<type>::max() - steps) // overflow
                new_value = m_maximum;
            else if (steps < 0 && m_value < std::numeric_limits<type>::min() - steps) // underflow
                new_value = m_minimum;
            else if ((m_value + steps) < m_minimum)
                new_value = m_minimum;
            else if ((m_value + steps) > m_maximum)
                new_value = m_maximum;
            else
                new_value = m_value + steps;

            setValue(new_value);
        }

      protected:
        // bool event(QEvent *event);
        QValidator::State validate(QString& input, int&) const override
        {
            if (input.isEmpty())
                return QValidator::Acceptable;

            // technically should check if the range allows positive / negative values
            if (input.length() == 1 && (input[0] == '+' || input[0] == '-'))
                return QValidator::Acceptable;

            bool ok;
            type val = input.toLongLong(&ok);
            if (!ok)
                return QValidator::Invalid;

            if (val < m_minimum || val > m_maximum)
                return QValidator::Invalid;

            return QValidator::Acceptable;
        }

        virtual type valueFromText(const QString& text) const
        {
            return text.toLongLong();
        }

        virtual QString textFromValue(type val) const
        {
            return QString::number(val);
        }
        // virtual void fixup(QString &str) const;

        virtual QAbstractSpinBox::StepEnabled stepEnabled() const
        {
            return StepUpEnabled | StepDownEnabled;
        }

      public Q_SLOTS:
        void setValue(type val)
        {
            if (m_value != val)
            {
                auto new_text = textFromValue(val);
                lineEdit()->setText(new_text);
                m_value = val;
                // emit valueChanged(val);
                emit valueChanged(new_text);
            }
        }

        void onEditFinished()
        {
            auto new_text = text();
            m_value = valueFromText(new_text);
            // emit valueChanged(m_value);
            emit valueChanged(new_text);
        }

      Q_SIGNALS:
        // void valueChanged(type v);
        void valueChanged(const QString& v);
    };

    class ULongLongSpinBox : public QAbstractSpinBox
    {
        Q_OBJECT

        using type = unsigned long long;

        type m_minimum;
        type m_maximum;
        type m_value;

      public:
        explicit ULongLongSpinBox(QWidget* parent = nullptr) : QAbstractSpinBox(parent)
        {
            setRange(std::numeric_limits<type>::min(), std::numeric_limits<type>::max());
            QObject::connect(lineEdit(), &QLineEdit::textEdited, this, &ULongLongSpinBox::onEditFinished);
        };
        ~ULongLongSpinBox(){};

        type value() const
        {
            return m_value;
        };

        type minimum() const
        {
            return m_minimum;
        };

        void setMinimum(type min)
        {
            m_minimum = min;
        }

        type maximum() const
        {
            return m_maximum;
        };

        void setMaximum(type max)
        {
            m_maximum = max;
        }

        void setRange(type min, type max)
        {
            setMinimum(min);
            setMaximum(max);
        }

        void stepBy(int steps) override
        {
            type new_value;

            if (steps > 0 && m_value > std::numeric_limits<type>::max() - steps) // overflow
                new_value = m_maximum;
            else if (steps < 0 && m_value < std::numeric_limits<type>::min() - steps) // underflow
                new_value = m_minimum;
            else if ((m_value + steps) < m_minimum)
                new_value = m_minimum;
            else if ((m_value + steps) > m_maximum)
                new_value = m_maximum;
            else
                new_value = m_value + steps;

            setValue(new_value);
        }

      protected:
        // bool event(QEvent *event);
        QValidator::State validate(QString& input, int&) const override
        {
            if (input.isEmpty())
                return QValidator::Acceptable;

            // technically should be more complicated, checking if the range allows positive / negative values
            if (input.length() == 1 && input[0] == '+')
                return QValidator::Acceptable;

            bool ok;
            type val = input.toULongLong(&ok);
            if (!ok)
                return QValidator::Invalid;

            if (val < m_minimum || val > m_maximum)
                return QValidator::Invalid;

            return QValidator::Acceptable;
        }

        virtual type valueFromText(const QString& text) const
        {
            return text.toULongLong();
        }

        virtual QString textFromValue(type val) const
        {
            return QString::number(val);
        }
        // virtual void fixup(QString &str) const;

        virtual QAbstractSpinBox::StepEnabled stepEnabled() const
        {
            return StepUpEnabled | StepDownEnabled;
        }

      public Q_SLOTS:
        void setValue(type val)
        {
            if (m_value != val)
            {
                auto new_text = textFromValue(val);
                lineEdit()->setText(new_text);
                m_value = val;
                // emit valueChanged(val);
                emit valueChanged(new_text);
            }
        }

        void onEditFinished()
        {
            auto new_text = text();
            m_value = valueFromText(new_text);
            // emit valueChanged(m_value);
            emit valueChanged(new_text);
        }

      Q_SIGNALS:
        // void valueChanged(type v);
        void valueChanged(const QString& v);
    };
} // namespace S2Plugin
