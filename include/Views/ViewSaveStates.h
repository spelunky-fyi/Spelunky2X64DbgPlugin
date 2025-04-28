#pragma once

#include <QWidget>

class QTableWidget;

namespace S2Plugin
{
    class ViewSaveStates : public QWidget
    {
        Q_OBJECT
      public:
        ViewSaveStates(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void cellClicked(int row, int column);
        void refreshSlots();

      private:
        QTableWidget* mMainTable;
    };
} // namespace S2Plugin
