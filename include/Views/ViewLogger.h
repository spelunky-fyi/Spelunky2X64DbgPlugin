#pragma once

#include <QLineEdit>
#include <QPushButton>
#include <QSize>
#include <QTabWidget>
#include <QTableView>
#include <QWidget>

namespace S2Plugin
{
    class Logger;
    class TableViewLogger;
    class WidgetSampling;
    class ItemModelLoggerSamples;

    class ViewLogger : public QWidget
    {
        Q_OBJECT

      public:
        explicit ViewLogger(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void samplingEnded();
        void fieldsChanged();

      private:
        Logger* mLogger;

        // TOP LAYOUT
        QLineEdit* mSamplePeriodLineEdit;
        QLineEdit* mDurationLineEdit;
        QPushButton* mStartButton;

        // TABS
        QTabWidget* mMainTabWidget;

        // TABLE
        TableViewLogger* mFieldsTableView;
        WidgetSampling* mSamplingWidget;

        // SAMPLES
        QTableView* mSamplesTableView;
        ItemModelLoggerSamples* mSamplesTableModel;

        void startLogging();
    };
} // namespace S2Plugin
