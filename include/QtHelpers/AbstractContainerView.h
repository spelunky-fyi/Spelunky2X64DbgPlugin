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
        AbstractContainerView(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;
        virtual void reloadContainer() = 0;

        TreeViewMemoryFields* mMainTreeView;
    };
} // namespace S2Plugin
