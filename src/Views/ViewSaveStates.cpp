#include "Views/ViewSaveStates.h"

#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <cstdint>

constexpr uint8_t gsSaveStates = 5;
enum Columns
{
    InUse,
    HeapBase,
    State,
    LevelGen,
    LiquidPhysics,
    SaveGame
};

S2Plugin::ViewSaveStates::ViewSaveStates(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("Save States");

    auto mainLayout = new QVBoxLayout(this);
    auto horLayout = new QHBoxLayout();
    auto autoRefresh = new WidgetAutorefresh(100, this);
    horLayout->addWidget(autoRefresh);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewSaveStates::refreshSlots);
    horLayout->addStretch();
    mainLayout->addLayout(horLayout);

    mMainTable = new QTableWidget(this);
    mMainTable->setAlternatingRowColors(true);
    mMainTable->verticalHeader()->hide();
    mMainTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mMainTable->verticalHeader()->setDefaultSectionSize(30);
    mMainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mMainTable->horizontalHeader()->setStretchLastSection(true);
    mMainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mMainTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mMainTable->setColumnCount(6);
    mMainTable->setRowCount(gsSaveStates);
    auto HTMLDelegate = new StyledItemDelegateHTML(this);
    mMainTable->setItemDelegate(HTMLDelegate);
    mMainTable->setHorizontalHeaderLabels({"In Use", "Heap Base", "State", "LevelGen", "Liquid Physics", "SaveGame"});
    mMainTable->setColumnWidth(Columns::InUse, 60);
    mMainTable->setColumnWidth(Columns::HeapBase, 130);
    mMainTable->setColumnWidth(Columns::State, 130);
    mMainTable->setColumnWidth(Columns::LevelGen, 130);
    mMainTable->setColumnWidth(Columns::LiquidPhysics, 130);
    mMainTable->setColumnWidth(Columns::SaveGame, 130);
    HTMLDelegate->setCenterVertically(true);
    QObject::connect(mMainTable, &QTableWidget::cellClicked, this, &ViewSaveStates::cellClicked);

    mainLayout->addWidget(mMainTable);
    refreshSlots();
    autoRefresh->toggleAutoRefresh(true);
}

constexpr uint32_t gsRoleMemoryAddress = Qt::UserRole + 1;

void S2Plugin::ViewSaveStates::refreshSlots()
{
    auto updateItem = [&](int row, int column, uintptr_t address)
    {
        QString text;
        if (address != 0)
            text = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", address);

        auto item = mMainTable->item(row, column);
        if (item == nullptr)
        {
            item = new QTableWidgetItem(text);
            mMainTable->setItem(row, column, item);
        }
        else
            item->setText(text);

        item->setData(gsRoleMemoryAddress, address);
    };

    uintptr_t heapOffsetSaveGame = 0;
    auto gm = Spelunky2::get()->get_GameManagerPtr(true);
    if (gm != 0)
        heapOffsetSaveGame = Script::Memory::ReadQword(Script::Memory::ReadQword(gm + 8));

    auto saveStatePtr = Spelunky2::get()->get_SaveStatesPtr();
    uint8_t emptySlots = Script::Memory::ReadByte(saveStatePtr);
    saveStatePtr += 0x10;
    for (uint8_t i = 0; i < gsSaveStates; ++i)
    {
        auto statePtr = Script::Memory::ReadQword(saveStatePtr + i * sizeof(uintptr_t));
        if (i >= emptySlots)
            mMainTable->setItem(i, Columns::InUse, new QTableWidgetItem("<font color='green'><b>Yes</b></font>"));
        else
            mMainTable->setItem(i, Columns::InUse, new QTableWidgetItem("<font color='#AAA'>No</font>"));

        updateItem(i, Columns::HeapBase, statePtr);
        updateItem(i, Columns::State, statePtr + Spelunky2::GAME_OFFSET::STATE);
        updateItem(i, Columns::LevelGen, statePtr + Spelunky2::GAME_OFFSET::LEVEL_GEN);
        updateItem(i, Columns::LiquidPhysics, statePtr + Spelunky2::GAME_OFFSET::LIQUID_ENGINE);
        updateItem(i, Columns::SaveGame, heapOffsetSaveGame == 0 ? 0 : statePtr + heapOffsetSaveGame);
    }
}

QSize S2Plugin::ViewSaveStates::sizeHint() const
{
    return QSize(750, 375);
}

QSize S2Plugin::ViewSaveStates::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewSaveStates::cellClicked(int row, int column)
{
    auto clickedItem = mMainTable->item(row, column);
    switch (column)
    {
        case Columns::HeapBase:
        {
            auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (addr != 0)
            {
                GuiDumpAt(addr);
                GuiShowCpu();
            }
            break;
        }
        case Columns::State:
        {
            auto statePtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (statePtr != 0)
                getToolbar()->showState(statePtr);
            break;
        }
        case Columns::LevelGen:
        {
            auto levelGenPtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (levelGenPtr != 0)
                getToolbar()->showLevelGen(levelGenPtr);
            break;
        }
        case Columns::LiquidPhysics:
        {
            auto liquidPhysicsPtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (liquidPhysicsPtr != 0)
                getToolbar()->showLiquidPhysics(liquidPhysicsPtr);
            break;
        }
        case Columns::SaveGame:
        {
            auto saveGamePtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (saveGamePtr != 0)
                getToolbar()->showSaveGame(saveGamePtr);
            break;
        }
    }
}
