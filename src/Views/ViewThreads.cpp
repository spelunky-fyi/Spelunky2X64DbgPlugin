#include "Views/ViewThreads.h"

#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtPlugin.h"
#include "Spelunky2.h"
#include "Views/ViewToolbar.h"
#include "pluginmain.h"
#include <QHeaderView>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

static const uint32_t gsColThreadName = 0;
static const uint32_t gsColTEBAddress = 1;
static const uint32_t gsColStateAddress = 2;

static const uint32_t gsRoleMemoryAddress = Qt::UserRole + 1;

S2Plugin::ViewThreads::ViewThreads(QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("Threads");

    auto mainLayout = new QVBoxLayout(this);
    auto horLayout = new QHBoxLayout();
    auto refreshButton = new QPushButton("Refresh", this);
    horLayout->addWidget(refreshButton);
    QObject::connect(refreshButton, &QPushButton::clicked, this, &ViewThreads::refreshThreads);
    horLayout->addStretch();
    mainLayout->addLayout(horLayout);

    mMainTable = new QTableWidget(this);
    mMainTable->setColumnCount(3);
    mMainTable->setAlternatingRowColors(true);
    mMainTable->verticalHeader()->hide();
    mMainTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    mMainTable->verticalHeader()->setDefaultSectionSize(30);
    mMainTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mMainTable->horizontalHeader()->setStretchLastSection(true);
    mMainTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mMainTable->setSelectionMode(QAbstractItemView::SingleSelection);
    auto HTMLDelegate = new StyledItemDelegateHTML(this);
    mMainTable->setItemDelegate(HTMLDelegate);
    HTMLDelegate->setCenterVertically(true);
    QObject::connect(mMainTable, &QTableWidget::cellClicked, this, &ViewThreads::cellClicked);

    mainLayout->addWidget(mMainTable);
    refreshThreads();
}

void S2Plugin::ViewThreads::refreshThreads()
{
    auto feedCodeOffset = 0x60;

    mMainTable->clear();
    mMainTable->setHorizontalHeaderLabels(QStringList() << "Thread ID"
                                                        << "TEB address"
                                                        << "State");
    mMainTable->setColumnWidth(gsColThreadName, 150);
    mMainTable->setColumnWidth(gsColTEBAddress, 150);

    THREADLIST threadList;
    DbgGetThreadList(&threadList);
    mMainTable->setRowCount(threadList.count);
    for (auto x = 0; x < threadList.count; ++x)
    {
        auto threadAllInfo = threadList.list[x];

        std::string threadName(threadAllInfo.BasicInfo.threadName);
        QString name;
        if (threadName.empty())
        {
            name = QString("%1").arg(threadAllInfo.BasicInfo.ThreadId);
        }
        else
        {
            name = QString("%1").arg(QString::fromStdString(threadName));
        }
        auto item = new QTableWidgetItem(name);
        mMainTable->setItem(x, gsColThreadName, item);

        auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
        auto teb = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", tebAddress);
        item = new QTableWidgetItem(teb);
        item->setData(gsRoleMemoryAddress, tebAddress);
        mMainTable->setItem(x, gsColTEBAddress, item);

        if (!Script::Memory::IsValidPtr(tebAddress + (11 * sizeof(size_t))))
        {
            continue;
        }
        auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(size_t)));
        if (!Script::Memory::IsValidPtr(tebAddress11Ptr))
        {
            continue;
        }
        auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
        auto heapBase = Script::Memory::ReadQword(tebAddress11Value + TEB_offset);
        if (!Script::Memory::IsValidPtr(heapBase))
        {
            continue;
        }
        auto statePtr = heapBase + 0x4A0; // hardcode for now
        if (!Script::Memory::IsValidPtr(statePtr))
        {
            continue;
        }
        auto feedCode = statePtr + feedCodeOffset;
        if (!Script::Memory::IsValidPtr(statePtr))
        {
            continue;
        }
        if (Script::Memory::ReadDword(feedCode) != 0xFEEDC0DE)
        {
            continue;
        }

        auto s = QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", statePtr);
        item = new QTableWidgetItem(s);
        item->setData(gsRoleMemoryAddress, statePtr);
        mMainTable->setItem(x, gsColStateAddress, item);
    }
}

QSize S2Plugin::ViewThreads::sizeHint() const
{
    return QSize(550, 375);
}

QSize S2Plugin::ViewThreads::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewThreads::cellClicked(int row, int column)
{
    auto clickedItem = mMainTable->item(row, column);
    if (column == gsColTEBAddress)
    {
        auto addr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
        if (addr != 0)
        {
            GuiDumpAt(addr);
            GuiShowCpu();
        }
    }
    else if (column == gsColStateAddress)
    {
        auto statePtr = clickedItem->data(gsRoleMemoryAddress).toULongLong();
        if (statePtr != 0)
            getToolbar()->showState(statePtr);
    }
}
