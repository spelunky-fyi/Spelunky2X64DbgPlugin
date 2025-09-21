#pragma once

#include "Configuration.h"
#include <QWidget>
#include <cstdint>
#include <vector>

class QTabWidget;
class QTableWidget;

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewStruct : public QWidget
    {
        Q_OBJECT
      public:
        ViewStruct(uintptr_t address, const std::vector<MemoryField>& fields, const std::string& name, QWidget* parent = nullptr);

      protected slots:
        virtual void updateData();

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

        TreeViewMemoryFields* mMainTreeView;
    };

    class ViewArray : public ViewStruct
    {
        Q_OBJECT
      public:
        ViewArray(uintptr_t address, std::string arrayTypeName, size_t num, std::string name, QWidget* parent = nullptr);
      protected slots:
        void pageListUpdate(std::pair<size_t, size_t> range);

      private:
        MemoryField mArray;
        uintptr_t mArrayAddress;
    };
    class ViewMatrix : public ViewStruct
    {
        Q_OBJECT
      public:
        ViewMatrix(uintptr_t address, std::string arrayTypeName, size_t row, size_t col, std::string name, QWidget* parent = nullptr);

      protected slots:
        void pageListUpdate(std::pair<size_t, size_t> range);
        void updateData() override;
        void indexClicked(int r, int c);

      private:
        MemoryField mMatrix;
        uintptr_t mMatrixAddress;
        QTabWidget* mTabs;
        QTableWidget* mTableWidget;
    };
} // namespace S2Plugin
