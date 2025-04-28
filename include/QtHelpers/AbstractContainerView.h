#pragma once

#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetPagination.h"
#include <QWidget>

namespace S2Plugin
{
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
        WidgetPagination* mPagination;
    };
} // namespace S2Plugin
