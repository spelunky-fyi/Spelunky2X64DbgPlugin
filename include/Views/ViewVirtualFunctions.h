#pragma once

#include <QWidget>
#include <cstdint>
#include <string>

class QLineEdit;
class QModelIndex;
class QTableView;

namespace S2Plugin
{
    class ViewVirtualFunctions : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualFunctions(uintptr_t address, const std::string& typeName, QWidget* parent = nullptr);

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
