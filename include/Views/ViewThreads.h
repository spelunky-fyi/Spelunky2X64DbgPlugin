#pragma once

#include <QSize>
#include <QTableWidget>
#include <QWidget>

namespace S2Plugin
{
    class ViewThreads : public QWidget
    {
        Q_OBJECT
      public:
        ViewThreads(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void cellClicked(int row, int column);
        void refreshThreads();

      private:
        QTableWidget* mMainTable;
    };
} // namespace S2Plugin
