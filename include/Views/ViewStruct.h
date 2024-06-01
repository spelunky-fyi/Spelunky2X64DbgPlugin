#pragma once

#include <QSize>
#include <QWidget>
#include <cstdint>
#include <vector>

namespace S2Plugin
{
    class TreeViewMemoryFields;
    struct MemoryField;

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
      public:
        ViewArray(uintptr_t address, std::string arrayTypeName, size_t num, std::string name, QWidget* parent = nullptr);
    };
    class ViewMatrix : public ViewStruct
    {
      public:
        ViewMatrix(uintptr_t address, std::string arrayTypeName, size_t row, size_t col, std::string name, QWidget* parent = nullptr);
    };
} // namespace S2Plugin
