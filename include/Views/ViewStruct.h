#pragma once

#include "Configuration.h"
#include <QWidget>
#include <cstdint>
#include <vector>

namespace S2Plugin
{
    class TreeViewMemoryFields;
    class WidgetPagination;

    class ViewStruct : public QWidget
    {
        Q_OBJECT
      public:
        ViewStruct(uintptr_t address, const std::vector<MemoryField>& fields, const std::string& name, QWidget* parent = nullptr);

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
        void pageListUpdate();

      private:
        WidgetPagination* mPagination;
        MemoryField mArray;
        uintptr_t mArrayAddress;
    };
    class ViewMatrix : public ViewStruct
    {
        Q_OBJECT
      public:
        ViewMatrix(uintptr_t address, std::string arrayTypeName, size_t row, size_t col, std::string name, QWidget* parent = nullptr);

      protected slots:
        void pageListUpdate();

      private:
        WidgetPagination* mPagination;
        MemoryField mMatrix;
        uintptr_t mMatrixAddress;
    };
} // namespace S2Plugin
