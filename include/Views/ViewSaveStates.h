#pragma once

#include <QWidget>
#include <cstdint>

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
        uintptr_t mSaveGameOffset{0};
    };
} // namespace S2Plugin
