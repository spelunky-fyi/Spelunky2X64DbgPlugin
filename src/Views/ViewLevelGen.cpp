#include "Views/ViewLevelGen.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetSpelunkyRooms.h"
#include "QtPlugin.h"
#include "pluginmain.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

S2Plugin::ViewLevelGen::ViewLevelGen(uintptr_t address, QWidget* parent) : QWidget(parent), mLevelGenPtr(address)
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
}

void S2Plugin::ViewLevelGen::initializeUI()
{
    auto mainLayout = new QVBoxLayout(this);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto autoRefresh = new WidgetAutorefresh("500", this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewLevelGen::refreshLevelGen);
    refreshLayout->addWidget(autoRefresh);

    refreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, this, &ViewLevelGen::label);
    refreshLayout->addWidget(labelButton);

    mMainTabWidget = new QTabWidget(this);
    mMainTabWidget->setDocumentMode(false);
    mainLayout->addWidget(mMainTabWidget);

    // TABS
    auto tabData = new QWidget();
    auto tabRooms = new QWidget();
    tabData->setLayout(new QVBoxLayout());
    tabData->layout()->setMargin(0);
    tabData->setObjectName("datawidget");
    tabRooms->setLayout(new QVBoxLayout());
    tabRooms->layout()->setMargin(0);

    mMainTabWidget->addTab(tabData, "Data");
    mMainTabWidget->addTab(tabRooms, "Rooms");

    // TAB DATA
    {
        mMainTreeView = new TreeViewMemoryFields(this);
        mMainTreeView->addMemoryFields(Configuration::get()->typeFields(MemoryFieldType::LevelGen), "LevelGen", mLevelGenPtr);
        tabData->layout()->addWidget(mMainTreeView);

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
        dynamic_cast<QVBoxLayout*>(tabRooms->layout())->addWidget(scroll);
    }

    mainLayout->setMargin(5);
    mMainTreeView->setVisible(true);
    autoRefresh->toggleAutoRefresh(true);
}

void S2Plugin::ViewLevelGen::refreshLevelGen()
{
    mMainTreeView->updateTree();

    if (mMainTabWidget->currentIndex() == 0)
    {
        auto offset = mLevelGenPtr;
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
    mMainTabWidget->setCurrentIndex(1);
}
