#pragma once

#include <QCloseEvent>
#include <QLineEdit>
#include <QModelIndex>
#include <QTableView>
#include <QWidget>

namespace S2Plugin
{
    class ViewVirtualFunctions : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualFunctions(const std::string& typeName, size_t offset, QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void tableEntryClicked(const QModelIndex& index);
        void jumpToFunction(bool b);

      private:
        std::string mTypeName;
        size_t mMemoryOffset;

        QLineEdit* mJumpToLineEdit;
        QTableView* mFunctionsTable;

        void initializeUI();
    };
} // namespace S2Plugin
