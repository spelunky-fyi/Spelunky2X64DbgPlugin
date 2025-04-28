#pragma once

#include <QWidget>

class QPushButton;
class QCheckBox;
class QTimer;
class QSpinBox;

namespace S2Plugin
{
    class WidgetAutorefresh : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetAutorefresh(int initialInterval, QWidget* parent = nullptr);
      signals:
        void refresh();

      public slots:
        void toggleAutoRefresh(bool checked);
      protected slots:
        void autoRefreshIntervalChanged(int val);

      private:
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QTimer* mAutoRefreshTimer;
        QSpinBox* mAutoRefreshIntervalQSpinBox;
    };
} // namespace S2Plugin
