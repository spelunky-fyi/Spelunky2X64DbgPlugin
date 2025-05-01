#pragma once

#include "Configuration.h"
#include <QDialog>
#include <QString>
#include <QWidget>
#include <cstdint>

class QLineEdit;
class QAbstractSpinBox;

namespace S2Plugin
{
    class DialogEditSimpleValue : public QDialog
    {
        Q_OBJECT
      public:
        explicit DialogEditSimpleValue(const QString& fieldName, uintptr_t memoryAddress, MemoryFieldType type, QWidget* parent = nullptr);

      protected:
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      private slots:
        void cancelBtnClicked();
        void changeBtnClicked();
        void decValueChanged(const QString& text);

      private:
        uintptr_t mMemoryAddress;
        MemoryFieldType mFieldType;

        QAbstractSpinBox* mSpinBox;
        QLineEdit* mLineEditHexValue;
    };
} // namespace S2Plugin
