#pragma once

#include <QSize>
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class AbstractContainerView : public QWidget
    {
        Q_OBJECT
      public:
        explicit AbstractContainerView(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        virtual void reloadContainer() = 0;

        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
