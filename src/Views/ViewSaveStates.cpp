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
#include <array>
#include <cstdint>
#include <vector>

constexpr uint8_t gsSaveStates = 5;
enum Column
{
    InUse,
    HeapBase,
    State,
    LevelGen,
    LiquidPhysics,
    SaveGame,
    Thread
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
    mMainTable->setColumnCount(7);
    mMainTable->setRowCount(gsSaveStates + 1);
    auto HTMLDelegate = new StyledItemDelegateHTML(this);
    mMainTable->setItemDelegate(HTMLDelegate);
    mMainTable->setHorizontalHeaderLabels({"In Use", "Heap Base", "State", "LevelGen", "Liquid Physics", "SaveGame", "Thread ID"});
    mMainTable->setColumnWidth(Column::InUse, 60);
    mMainTable->setColumnWidth(Column::HeapBase, 130);
    mMainTable->setColumnWidth(Column::State, 130);
    mMainTable->setColumnWidth(Column::LevelGen, 130);
    mMainTable->setColumnWidth(Column::LiquidPhysics, 130);
    mMainTable->setColumnWidth(Column::SaveGame, 130);
    mMainTable->setColumnWidth(Column::Thread, 130);
    HTMLDelegate->setCenterVertically(true);
    QObject::connect(mMainTable, &QTableWidget::cellClicked, this, &ViewSaveStates::cellClicked);

    mainLayout->addWidget(mMainTable);
    refreshSlots();
    autoRefresh->toggleAutoRefresh(true);
}

constexpr uint32_t gsRoleMemoryAddress = Qt::UserRole + 1;

static std::vector<std::pair<int, uintptr_t>> get_AllHeapBases()
{
    std::vector<std::pair<int, uintptr_t>> heapBaseList;
    THREADLIST threadList;
    DbgGetThreadList(&threadList);

    for (int x = 0; x < threadList.count; ++x)
    {
        auto threadAllInfo = threadList.list[x];
        auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
        auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(uintptr_t)));
        auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
        auto heapBasePtr = Script::Memory::ReadQword(tebAddress11Value + S2Plugin::TEB_offset);
        if (!Script::Memory::IsValidPtr(heapBasePtr))
            continue;

        heapBaseList.emplace_back(threadAllInfo.BasicInfo.ThreadNumber, heapBasePtr);
    }
    return heapBaseList;
};

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

    if (mSaveGameOffset == 0)
    {
        auto gm = Spelunky2::get()->get_GameManagerPtr(true);
        if (gm != 0)
            mSaveGameOffset = Script::Memory::ReadQword(Script::Memory::ReadQword(gm + 8));
    }
    auto saveStatePtr = Spelunky2::get()->get_SaveStatesPtr();
    uint8_t emptySlots = Script::Memory::ReadByte(saveStatePtr);
    saveStatePtr += 0x10;

    std::array<uintptr_t, gsSaveStates> saveStates = {};
    Script::Memory::Read(saveStatePtr, saveStates.data(), gsSaveStates * sizeof(uintptr_t), nullptr);
    auto heapBases = get_AllHeapBases();

    for (uint8_t i = 0; i < gsSaveStates; ++i)
    {
        const auto statePtr = saveStates[i];
        if (i >= emptySlots)
            mMainTable->setItem(i, Column::InUse, new QTableWidgetItem("<font color='green'><b>Yes</b></font>"));
        else
            mMainTable->setItem(i, Column::InUse, new QTableWidgetItem("<font color='#AAA'>No</font>"));

        updateItem(i, Column::HeapBase, statePtr);
        updateItem(i, Column::State, statePtr + Spelunky2::GAME_OFFSET::STATE);
        updateItem(i, Column::LevelGen, statePtr + Spelunky2::GAME_OFFSET::LEVEL_GEN);
        updateItem(i, Column::LiquidPhysics, statePtr + Spelunky2::GAME_OFFSET::LIQUID_ENGINE);
        updateItem(i, Column::SaveGame, mSaveGameOffset == 0 ? 0 : statePtr + mSaveGameOffset);

        auto it = heapBases.begin();
        for (; it != heapBases.end(); ++it)
            if (it->second == statePtr)
                break;

        QString text;
        if (it == heapBases.end())
            text = "No thread?";
        else if (it->first == 0)
            text = "Main";
        else
            text = QString::number(it->first);

        auto item = mMainTable->item(i, Column::Thread);
        if (item == nullptr)
        {
            item = new QTableWidgetItem(text);
            mMainTable->setItem(i, Column::Thread, item);
        }
        else
            item->setText(text);
    }

    auto hb = Spelunky2::get()->get_HeapBase(true);
    mMainTable->setItem(5, Column::InUse, new QTableWidgetItem("Main"));
    updateItem(5, Column::HeapBase, hb);
    updateItem(5, Column::State, hb + Spelunky2::GAME_OFFSET::STATE);
    updateItem(5, Column::LevelGen, hb + Spelunky2::GAME_OFFSET::LEVEL_GEN);
    updateItem(5, Column::LiquidPhysics, hb + Spelunky2::GAME_OFFSET::LIQUID_ENGINE);
    updateItem(5, Column::SaveGame, mSaveGameOffset == 0 ? 0 : hb + mSaveGameOffset);
    mMainTable->setItem(5, Column::Thread, new QTableWidgetItem("Main"));
}

QSize S2Plugin::ViewSaveStates::sizeHint() const
{
    return QSize(850, 375);
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
        case Column::HeapBase:
        {
            auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (addr != 0)
            {
                GuiDumpAt(addr);
                GuiShowCpu();
            }
            break;
        }
        case Column::State:
        {
            auto statePtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (statePtr != 0)
                getToolbar()->showState(statePtr);
            break;
        }
        case Column::LevelGen:
        {
            auto levelGenPtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (levelGenPtr != 0)
                getToolbar()->showLevelGen(levelGenPtr);
            break;
        }
        case Column::LiquidPhysics:
        {
            auto liquidPhysicsPtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (liquidPhysicsPtr != 0)
                getToolbar()->showLiquidPhysics(liquidPhysicsPtr);
            break;
        }
        case Column::SaveGame:
        {
            auto saveGamePtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
            if (saveGamePtr != 0)
                getToolbar()->showSaveGame(saveGamePtr);
            break;
        }
    }
}
