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
        ViewStruct(uintptr_t address, const std::vector<MemoryField>& fields, const std::string name, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private:
        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
