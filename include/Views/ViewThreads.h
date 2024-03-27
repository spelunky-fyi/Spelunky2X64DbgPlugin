#pragma once

#include "QtHelpers/StyledItemDelegateHTML.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

namespace S2Plugin
{
    class ViewToolbar;

    class ViewThreads : public QWidget
    {
        Q_OBJECT
      public:
        ViewThreads(ViewToolbar* toolbar);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void cellClicked(int row, int column);
        void refreshThreads();

      private:
        ViewToolbar* mToolbar;
        StyledItemDelegateHTML mHTMLDelegate;

        QVBoxLayout* mMainLayout;
        QTableWidget* mMainTable;

        void initializeUI();
    };
} // namespace S2Plugin
