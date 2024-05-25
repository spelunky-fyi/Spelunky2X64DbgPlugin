#pragma once

#include <QLineEdit>
#include <QModelIndex>
#include <QSize>
#include <QTableView>
#include <QWidget>
#include <cstdint>

namespace S2Plugin
{
    class ViewVirtualFunctions : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualFunctions(const std::string& typeName, uintptr_t address, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void tableEntryClicked(const QModelIndex& index);
        void jumpToFunction();

      private:
        uintptr_t mMemoryAddress;

        QLineEdit* mJumpToLineEdit;
        QTableView* mFunctionsTable;
    };
} // namespace S2Plugin
