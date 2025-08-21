#include "Views/ViewLevelGen.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetSpelunkyRooms.h"
#include "QtPlugin.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

S2Plugin::ViewLevelGen::ViewLevelGen(uintptr_t address, QWidget* parent) : QWidget(parent), mLevelGenPtr(address)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("LevelGen");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto autoRefresh = new WidgetAutorefresh(500, this);
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
    mMainTreeView = new TreeViewMemoryFields(this);
    auto tabRooms = new QScrollArea(this);

    mMainTabWidget->addTab(mMainTreeView, "Data");
    mMainTabWidget->addTab(tabRooms, "Rooms");

    // TAB DATA
    {
        mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
        mMainTreeView->updateTableHeader(false);
        mMainTreeView->setColumnWidth(gsColValue, 160);
        mMainTreeView->setColumnWidth(gsColField, 200);
        mMainTreeView->setColumnWidth(gsColValueHex, 125);
        mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
        mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
        mMainTreeView->setColumnWidth(gsColType, 100);
        mMainTreeView->addMemoryFields(Configuration::get()->typeFields(MemoryFieldType::LevelGen), "LevelGen", mLevelGenPtr);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::levelGenRoomsPointerClicked, this, &ViewLevelGen::levelGenRoomsPointerClicked);
        QObject::connect(mMainTreeView, &TreeViewMemoryFields::onContextMenu, this, &ViewLevelGen::viewContextMenu);
    }

    // TAB ROOMS
    {
        tabRooms->setWidgetResizable(true);
        auto containerWidget = new QWidget(this);
        tabRooms->setWidget(containerWidget);
        auto containerLayout = new QVBoxLayout(containerWidget);
        auto checkBoxLayout = new QHBoxLayout();
        auto enumCheckBox = new QCheckBox("Enum names");
        QObject::connect(enumCheckBox, &QCheckBox::stateChanged, this,
                         [this, enumCheckBox, containerWidget]
                         {
                             mUseEnumNames = enumCheckBox->isChecked();
                             containerWidget->update();
                         });

        checkBoxLayout->addWidget(enumCheckBox);
        auto pathCheckBox = new QCheckBox("Show Path");
        QObject::connect(pathCheckBox, &QCheckBox::stateChanged, this,
                         [this, pathCheckBox, containerWidget]
                         {
                             mShowPath = pathCheckBox->isChecked();
                             containerWidget->update();
                         });
        checkBoxLayout->addWidget(pathCheckBox);
        containerLayout->addLayout(checkBoxLayout);

        bool once = false;
        for (const auto& field : Configuration::get()->typeFields(MemoryFieldType::LevelGen))
        {
            if (field.type == MemoryFieldType::LevelGenRoomsPointer || field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
            {
                auto roomWidget = new WidgetSpelunkyRooms(field.name, this);
                QObject::connect(tabRooms->verticalScrollBar(), &QScrollBar::valueChanged, roomWidget, &WidgetSpelunkyRooms::resetHover);
                if (field.type == MemoryFieldType::LevelGenRoomsMetaPointer)
                    roomWidget->setAsMetaData();
                else
                {
                    roomWidget->setNameSwitch(&mUseEnumNames);
                    if (!once)
                    {
                        roomWidget->setPathVisible(&mShowPath);
                        once = true;
                    }
                }

                mRoomsWidgets[field.name] = roomWidget;
                containerLayout->addWidget(roomWidget);
            }
        }
    }
    autoRefresh->toggleAutoRefresh(true);
    mMainTreeView->updateTree(0, 0, true);
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
void S2Plugin::ViewLevelGen::viewContextMenu(QMenu* menu)
{
    auto action = menu->addAction("View Code");
    QObject::connect(action, &QAction::triggered, menu, [this]() { getToolbar()->showCode("LevelGen"); });
}
