#pragma once

#include <QSize>
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewEntityList : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntityList(uintptr_t address, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntityListContents();

      private:
        uintptr_t mEntityListAddress;

        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
