#pragma once

#include "Data/Logger.h"
#include "QtHelpers/ItemModelLoggerFields.h"
#include "QtHelpers/ItemModelLoggerSamples.h"
#include "QtHelpers/TableViewLogger.h"
#include "QtHelpers/WidgetSamplesPlot.h"
#include "QtHelpers/WidgetSampling.h"
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>


namespace S2Plugin
{
    class ViewLogger : public QWidget
    {
        Q_OBJECT

      public:
        explicit ViewLogger(QWidget* parent = nullptr);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void samplingEnded();
        void fieldsChanged();

      private:
        QVBoxLayout* mMainLayout;
        std::unique_ptr<Logger> mLogger;

        // TOP LAYOUT
        QHBoxLayout* mTopLayout;
        QLineEdit* mSamplePeriodLineEdit;
        QLineEdit* mDurationLineEdit;
        QPushButton* mStartButton;

        // TABS
        QTabWidget* mMainTabWidget;
        QWidget* mTabFields;
        QWidget* mTabSamples;
        QWidget* mTabPlot;

        // TABLE
        TableViewLogger* mFieldsTableView;
        ItemModelLoggerFields* mFieldsTableModel;
        WidgetSampling* mSamplingWidget;

        // SAMPLES
        QTableView* mSamplesTableView;
        ItemModelLoggerSamples* mSamplesTableModel;

        // PLOT
        QScrollArea* mSamplesPlotScroll;
        WidgetSamplesPlot* mSamplesPlotWidget;

        void initializeUI();
        void startLogging();
    };
} // namespace S2Plugin
