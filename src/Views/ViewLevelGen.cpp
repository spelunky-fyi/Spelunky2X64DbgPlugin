#include "Views/ViewLevelGen.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetSpelunkyRooms.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>

S2Plugin::ViewLevelGen::ViewLevelGen(QWidget* parent) : QWidget(parent)
{
    initializeUI();
    setWindowIcon(getCavemanIcon());
    setWindowTitle("LevelGen");
    mMainTreeView->updateTree(0, 0, true);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    toggleAutoRefresh(Qt::Checked);
}

void S2Plugin::ViewLevelGen::initializeUI()
{
    mMainLayout = new QVBoxLayout(this);
    mRefreshLayout = new QHBoxLayout();
    mMainLayout->addLayout(mRefreshLayout);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mMainLayout->addWidget(mMainTabWidget);

    // TOP REFRESH LAYOUT
    mRefreshButton = new QPushButton("Refresh", this);
    mRefreshLayout->addWidget(mRefreshButton);
    QObject::connect(mRefreshButton, &QPushButton::clicked, this, &ViewLevelGen::refreshLevelGen);

    mAutoRefreshTimer = std::make_unique<QTimer>(this);
    QObject::connect(mAutoRefreshTimer.get(), &QTimer::timeout, this, &ViewLevelGen::refreshLevelGen);

    mAutoRefreshCheckBox = new QCheckBox("Auto-refresh every", this);
    mAutoRefreshCheckBox->setCheckState(Qt::Checked);
    mRefreshLayout->addWidget(mAutoRefreshCheckBox);
    QObject::connect(mAutoRefreshCheckBox, &QCheckBox::clicked, this, &ViewLevelGen::toggleAutoRefresh);

    mAutoRefreshIntervalLineEdit = new QLineEdit(this);
    mAutoRefreshIntervalLineEdit->setFixedWidth(50);
    mAutoRefreshIntervalLineEdit->setValidator(new QIntValidator(100, 5000, this));
    mAutoRefreshIntervalLineEdit->setText("500");
    mRefreshLayout->addWidget(mAutoRefreshIntervalLineEdit);
    QObject::connect(mAutoRefreshIntervalLineEdit, &QLineEdit::textChanged, this, &ViewLevelGen::autoRefreshIntervalChanged);

    mRefreshLayout->addWidget(new QLabel("milliseconds", this));

    mRefreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewLevelGen::label);
    mRefreshLayout->addWidget(labelButton);

    // TABS
    mTabData = new QWidget();
    mTabRooms = new QWidget();
    mTabData->setLayout(new QVBoxLayout(mTabData));
    mTabData->layout()->setMargin(0);
    mTabData->setObjectName("datawidget");
    mTabRooms->setLayout(new QVBoxLayout(mTabRooms));
    mTabRooms->layout()->setMargin(0);

    mMainTabWidget->addTab(mTabData, "Data");
    mMainTabWidget->addTab(mTabRooms, "Rooms");

    // TAB DATA
    {
        mMainTreeView = new TreeViewMemoryFields(this);
        mMainTreeView->addMemoryFields(Configuration::get()->typeFields(MemoryFieldType::LevelGen), "LevelGen", Spelunky2::get()->get_LevelGenPtr());
        mTabData->layout()->addWidget(mMainTreeView);

        mMainTreeView->setColumnWidth(gsColValue, 250);
        mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader();
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::levelGenRoomsPointerClicked, this, &ViewLevelGen::levelGenRoomsPointerClicked);
    }

    // TAB ROOMS
    {
        auto scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        auto containerWidget = new QWidget(this);
        scroll->setWidget(containerWidget);
        auto containerLayout = new QVBoxLayout(containerWidget);

        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
        {
            if (field.type == MemoryFieldType::LevelGenRoomsPointer || field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
            {
                auto roomWidget = new WidgetSpelunkyRooms(field.name, this);
                if (field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
                {
                    roomWidget->setIsMetaData();
                }
                mRoomsWidgets[field.name] = roomWidget;
                containerLayout->addWidget(roomWidget);
            }
        }
        dynamic_cast<QVBoxLayout*>(mTabRooms->layout())->addWidget(scroll);
    }

    mMainLayout->setMargin(5);
    mMainTreeView->setVisible(true);
}

void S2Plugin::ViewLevelGen::refreshLevelGen()
{
    mMainTreeView->updateTree();

    if (mMainTabWidget->currentWidget() == mTabRooms)
    {
        auto offset = Spelunky2::get()->get_LevelGenPtr(); // TODO: save ptr to sturct on view open
        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
        {
            if (field.type == MemoryFieldType::LevelGenRoomsPointer || field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
            {
                size_t pointerOffset = Script::Memory::ReadQword(offset);
                mRoomsWidgets.at(field.name)->setOffset(pointerOffset);
            }
            offset += field.get_size();
        }
    }
}

void S2Plugin::ViewLevelGen::toggleAutoRefresh(int newLevelGen)
{
    if (newLevelGen == Qt::Unchecked)
    {
        mAutoRefreshTimer->stop();
        mRefreshButton->setEnabled(true);
    }
    else
    {
        mAutoRefreshTimer->setInterval(mAutoRefreshIntervalLineEdit->text().toUInt());
        mAutoRefreshTimer->start();
        mRefreshButton->setEnabled(false);
    }
}

void S2Plugin::ViewLevelGen::autoRefreshIntervalChanged(const QString& text)
{
    if (mAutoRefreshCheckBox->checkState() == Qt::Checked)
    {
        mAutoRefreshTimer->setInterval(text.toUInt());
    }
}

QSize S2Plugin::ViewLevelGen::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewLevelGen::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewLevelGen::label()
{
    mMainTreeView->labelAll();
}

void S2Plugin::ViewLevelGen::levelGenRoomsPointerClicked()
{
    mMainTabWidget->setCurrentWidget(mTabRooms);
}
