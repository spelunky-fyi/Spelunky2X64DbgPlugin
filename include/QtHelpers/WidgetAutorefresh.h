#pragma once

#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTimer>
#include <QWidget>

namespace S2Plugin
{
    class WidgetAutorefresh : public QWidget
    {
        Q_OBJECT
      public:
        WidgetAutorefresh(const QString& initialInterval, QWidget* parrent = nullptr);
      signals:
        void refresh();

      public slots:
        void toggleAutoRefresh(bool checked);
      protected slots:
        void autoRefreshIntervalChanged(const QString& text);

      private:
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QTimer* mAutoRefreshTimer;
        QLineEdit* mAutoRefreshIntervalLineEdit;
    };
} // namespace S2Plugin
