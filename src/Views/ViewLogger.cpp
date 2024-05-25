#include "Views/ViewLogger.h"

#include "Data/Logger.h"
#include "QtHelpers/ItemModelLoggerFields.h"
#include "QtHelpers/ItemModelLoggerSamples.h"
#include "QtHelpers/TableViewLogger.h"
#include "QtHelpers/WidgetSamplesPlot.h"
#include "QtHelpers/WidgetSampling.h"
#include "QtPlugin.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

S2Plugin::ViewLogger::ViewLogger(QWidget* parent) : QWidget(parent)
{
    mLogger = new Logger(this);

    setWindowIcon(getCavemanIcon());
    setWindowTitle("Logger");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);

    auto topLayout = new QHBoxLayout(this);
    topLayout->addWidget(new QLabel("Sample period:", this));
    mSamplePeriodLineEdit = new QLineEdit("8", this);
    mSamplePeriodLineEdit->setFixedWidth(50);
    mSamplePeriodLineEdit->setValidator(new QIntValidator(5, 5000, this));
    topLayout->addWidget(mSamplePeriodLineEdit);
    topLayout->addWidget(new QLabel("milliseconds", this));

    topLayout->addStretch();

    topLayout->addWidget(new QLabel("Duration:", this));
    mDurationLineEdit = new QLineEdit("5", this);
    topLayout->addWidget(mDurationLineEdit);
    mDurationLineEdit->setFixedWidth(50);
    mDurationLineEdit->setValidator(new QIntValidator(1, 500, this));
    topLayout->addWidget(new QLabel("seconds", this));

    topLayout->addStretch();

    mStartButton = new QPushButton(this);
    mStartButton->setText("Start");
    topLayout->addWidget(mStartButton);
    QObject::connect(mStartButton, &QPushButton::clicked, this, &ViewLogger::startLogging);

    mainLayout->addLayout(topLayout);

    // TABS
    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    auto tabFields = new QWidget();
    auto tabSamples = new QWidget();
    auto tabPlot = new QWidget();
    tabFields->setLayout(new QVBoxLayout());
    tabFields->layout()->setMargin(0);
    tabSamples->setLayout(new QVBoxLayout());
    tabSamples->layout()->setMargin(0);
    tabPlot->setLayout(new QVBoxLayout());
    tabPlot->layout()->setMargin(0);

    mMainTabWidget->addTab(tabFields, "Fields");
    mMainTabWidget->addTab(tabSamples, "Samples");
    mMainTabWidget->addTab(tabPlot, "Plot");

    // TAB Fields
    {
        mFieldsTableView = new TableViewLogger(mLogger, this);
        mFieldsTableView->setDragDropMode(QAbstractItemView::DragDropMode::DropOnly);
        mFieldsTableView->setAcceptDrops(true);
        tabFields->layout()->addWidget(mFieldsTableView);

        auto fieldsTableModel = new ItemModelLoggerFields(mLogger, mFieldsTableView);
        mLogger->setTableModel(fieldsTableModel);
        mFieldsTableView->setModel(fieldsTableModel);
        mFieldsTableView->setColumnWidth(gsLogFieldColColor, 45);
        mFieldsTableView->setColumnWidth(gsLogFieldColMemoryOffset, 125);
        mFieldsTableView->setColumnWidth(gsLogFieldColFieldType, 150);
    }

    // TAB Samples
    {
        mSamplesTableView = new QTableView(this);
        tabSamples->layout()->addWidget(mSamplesTableView);
        mSamplesTableModel = new ItemModelLoggerSamples(mLogger, mSamplesTableView);
        mSamplesTableView->setModel(mSamplesTableModel);
    }

    // TAB Plot
    {
        auto samplesPlotScroll = new QScrollArea(this);
        auto samplesPlotWidget = new WidgetSamplesPlot(mLogger, this);
        samplesPlotScroll->setBackgroundRole(QPalette::Dark);
        samplesPlotScroll->setWidget(samplesPlotWidget);
        samplesPlotScroll->setWidgetResizable(true);
        tabPlot->layout()->addWidget(samplesPlotScroll);
    }

    QObject::connect(mLogger, &Logger::samplingEnded, this, &ViewLogger::samplingEnded);
    QObject::connect(mLogger, &Logger::fieldsChanged, this, &ViewLogger::fieldsChanged);

    mSamplingWidget = new WidgetSampling(this);
    mSamplingWidget->setHidden(true);
    mainLayout->addWidget(mSamplingWidget);
}

QSize S2Plugin::ViewLogger::sizeHint() const
{
    return QSize(650, 450);
}

QSize S2Plugin::ViewLogger::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewLogger::startLogging()
{
    if (mLogger->fieldCount() > 0)
    {
        mSamplePeriodLineEdit->setEnabled(false);
        mDurationLineEdit->setEnabled(false);
        mStartButton->setEnabled(false);
        mMainTabWidget->setHidden(true);
        mSamplingWidget->setHidden(false);
        mLogger->start(mSamplePeriodLineEdit->text().toInt(), mDurationLineEdit->text().toInt());
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowIcon(getCavemanIcon());
        msgBox.setText("Please specify one or more fields to log");
        msgBox.setWindowTitle("Spelunky2");
        msgBox.exec();
    }
}

void S2Plugin::ViewLogger::samplingEnded()
{
    mSamplePeriodLineEdit->setEnabled(true);
    mDurationLineEdit->setEnabled(true);
    mStartButton->setEnabled(true);
    mSamplingWidget->setHidden(true);
    mMainTabWidget->setHidden(false);
    mSamplesTableModel->reset();
}

void S2Plugin::ViewLogger::fieldsChanged()
{
    mSamplesTableModel->reset();
}
