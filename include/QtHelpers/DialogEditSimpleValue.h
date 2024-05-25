#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSize>
#include <QSpinBox>
#include <QString>
#include <QWidget>
#include <cstdint>

namespace S2Plugin
{
    enum class MemoryFieldType;

    class DialogEditSimpleValue : public QDialog
    {
        Q_OBJECT

      public:
        DialogEditSimpleValue(const QString& fieldName, uintptr_t memoryAddress, MemoryFieldType type, QWidget* parent = nullptr);

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
