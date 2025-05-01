#pragma once

#include "Configuration.h"
#include <QDialog>
#include <QString>
#include <QWidget>
#include <cstdint>

class QLineEdit;

namespace S2Plugin
{
    class DialogEditString : public QDialog
    {
        Q_OBJECT
      public:
        // size without the last char (null termination)
        explicit DialogEditString(const QString& fieldName, QString initialValue, uintptr_t memoryAddress, int size, MemoryFieldType type, QWidget* parent = nullptr);

      protected:
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      private slots:
        void cancelBtnClicked();
        void changeBtnClicked();

      private:
        QLineEdit* mLineEdit;
        uintptr_t mMemoryAddress;
        MemoryFieldType mFieldType;
    };
} // namespace S2Plugin
