#include "Views/ViewStruct.h"

#include "Configuration.h"
#include "QtHelpers/StyledItemDelegateHTML.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetPagination.h"
#include "QtPlugin.h"
#include "Views/ViewToolbar.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>

S2Plugin::ViewStruct::ViewStruct(uintptr_t address, const std::vector<MemoryField>& fields, const std::string& name, QWidget* parent) : QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle(QString::fromStdString(name));
    mMainTreeView = new TreeViewMemoryFields(this);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto autoRefresh = new WidgetAutorefresh(100, this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, this, &ViewStruct::updateData);
    refreshLayout->addWidget(autoRefresh);

    refreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, mMainTreeView, static_cast<void (TreeViewMemoryFields::*)()>(&TreeViewMemoryFields::labelAll));
    refreshLayout->addWidget(labelButton);

    mainLayout->addWidget(mMainTreeView);
    mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColField, 200);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->addMemoryFields(fields, name, address);
    mMainTreeView->updateTree(0, 0, true);

    std::string nameCopy = name;
    QObject::connect(mMainTreeView, &TreeViewMemoryFields::onContextMenu, this,
                     [nameCopy, this](QMenu* menu)
                     {
                         auto action = menu->addAction("View Code");
                         QObject::connect(action, &QAction::triggered, menu, [nameCopy]() { getToolbar()->showCode(nameCopy); });
                     });

    autoRefresh->toggleAutoRefresh(true);
}

QSize S2Plugin::ViewStruct::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewStruct::minimumSizeHint() const
{
    return QSize(150, 150);
}

void S2Plugin::ViewStruct::updateData()
{
    mMainTreeView->updateTree();
}

void S2Plugin::ViewStruct::setStorage(uintptr_t addr, size_t s)
{
    mMainTreeView->setStorage(addr, s);
}

S2Plugin::ViewArray::ViewArray(uintptr_t address, std::string arrayTypeName, size_t num, std::string name, QWidget* parent)
    : ViewStruct(0, {}, arrayTypeName + " " + name + '[' + std::to_string(num) + ']', parent), mArrayAddress(address)
{

    mArray.name = name;
    mArray.type = MemoryFieldType::Array;
    mArray.firstParameterType = arrayTypeName;
    mArray.secondParameterType = '#'; // just to let it know it should put all the elements in, no array element
    mArray.numberOfElements = num;

    mMainTreeView->setStorage(address, mArray.get_size());

    auto pagination = new WidgetPagination(this);
    layout()->addWidget(pagination);
    QObject::connect(pagination, &WidgetPagination::pageUpdate, this, &ViewArray::pageListUpdate);
    pagination->setSize(num);
    pageListUpdate(pagination->getRange());
}

void S2Plugin::ViewArray::pageListUpdate(std::pair<size_t, size_t> range)
{
    mMainTreeView->clear();
    mMainTreeView->updateTableHeader();
    // using columns to store the initial index
    mArray.setNumColumns(range.first);
    mArray.numberOfElements = range.second;
    mMainTreeView->addMemoryField(mArray, {}, mArrayAddress, 0);
}

S2Plugin::ViewMatrix::ViewMatrix(uintptr_t address, std::string arrayTypeName, size_t rows, size_t columns, std::string name, QWidget* parent)
    : ViewStruct(0, {}, arrayTypeName + " " + name + '[' + std::to_string(rows) + "][" + std::to_string(columns) + ']', parent), mMatrixAddress(address)
{
    mMatrix.name = name;
    mMatrix.type = MemoryFieldType::Matrix;
    mMatrix.firstParameterType = arrayTypeName;
    mMatrix.rows = rows;
    mMatrix.setNumColumns(columns);

    mMainTreeView->setStorage(address, mMatrix.get_size());

    // hack in tab widget
    mTabs = new QTabWidget(this);
    layout()->replaceWidget(mMainTreeView, mTabs);
    mTabs->addTab(mMainTreeView, "Memory View");
    mTableWidget = new QTableWidget(this);
    auto columnsInt = static_cast<int>(columns);
    mTableWidget->setColumnCount(columnsInt);
    for (int idx = 0; idx < columnsInt; ++idx)
        mTableWidget->setHorizontalHeaderItem(idx, new QTableWidgetItem(QString("%1").arg(idx)));

    mTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    auto HTMLDelegate = new StyledItemDelegateHTML(this);
    HTMLDelegate->setCenterVertically(true);
    mTableWidget->setItemDelegate(HTMLDelegate);
    mTableWidget->setWordWrap(false);
    QObject::connect(mTableWidget, &QTableWidget::cellClicked, this, &ViewMatrix::indexClicked);
    mTabs->addTab(mTableWidget, "Table View");
    auto pagination = new WidgetPagination(this);
    layout()->addWidget(pagination);
    QObject::connect(pagination, &WidgetPagination::pageUpdate, this, &ViewMatrix::pageListUpdate);
    pagination->setSize(rows);
    pageListUpdate(pagination->getRange());
}

void S2Plugin::ViewMatrix::pageListUpdate(std::pair<size_t, size_t> range)
{
    mMainTreeView->clear();
    mMainTreeView->updateTableHeader();
    // dirty way to transfer the first index
    mMatrix.secondParameterType = std::to_string(range.first);
    mMatrix.rows = range.second;
    auto rowCount = static_cast<int>(range.second - range.first);

    mTableWidget->setRowCount(rowCount);
    for (int idx = 0; idx < rowCount; ++idx)
        mTableWidget->setVerticalHeaderItem(idx, new QTableWidgetItem(QString("%1").arg(idx + static_cast<int>(range.first))));

    mMainTreeView->addMemoryField(mMatrix, {}, mMatrixAddress, 0);
}

void S2Plugin::ViewMatrix::updateData()
{
    if (mTabs->currentIndex() == 1)
    {
        mMainTreeView->updateTree(mMatrixAddress);
        // update table
        auto treeModel = mMainTreeView->model();
        bool isCellStruct = [treeModel]()
        {
            auto tempIndex = treeModel->index(0, 0);
            if (!tempIndex.isValid())
                return false;

            tempIndex = tempIndex.child(0, gsColValue);
            if (!tempIndex.isValid())
                return false;

            auto textValue = tempIndex.data(Qt::DisplayRole).toString();
            // Checking text instead of type so it includes all the other types that just display the Expand/Collapse text instead of value
            if (textValue == "<font color='darkMagenta'><u>[Collapse]</u></font>" || textValue == "<font color='#AAA'><u>[Expand]</u></font>")
                return true;

            return false;
        }();

        for (int idx = 0; idx < treeModel->rowCount(); ++idx)
        {
            auto rowIndex = treeModel->index(idx, 0);
            auto columnIndex = rowIndex.child(0, gsColValue);
            while (columnIndex.isValid())
            {
                auto textValue = columnIndex.data(Qt::DisplayRole).toString();
                if (isCellStruct) // special fix for liquids_by_third_of_tile, let's hope I won't regret this in the future
                {
                    textValue.clear();
                    auto tempIndex = columnIndex.sibling(columnIndex.row(), gsColField).child(0, gsColValue);
                    while (tempIndex.isValid())
                    {
                        if (textValue.size() != 0)
                            textValue += ';';

                        textValue += tempIndex.data(Qt::DisplayRole).toString();
                        tempIndex = tempIndex.sibling(tempIndex.row() + 1, gsColValue);
                    }
                }

                auto item = mTableWidget->item(idx, columnIndex.row());
                if (item == nullptr)
                {
                    QTableWidgetItem* newItem = new QTableWidgetItem(textValue);
                    mTableWidget->setItem(idx, columnIndex.row(), newItem);
                }
                else
                    item->setText(textValue);

                columnIndex = rowIndex.child(columnIndex.row() + 1, gsColValue);
            }
        }
    }
    else
        mMainTreeView->updateTree();
}

void S2Plugin::ViewMatrix::indexClicked(int r, int c)
{
    auto index = mMainTreeView->model()->index(r, gsColField);
    if (!index.isValid())
        return;

    index = index.child(c, gsColValue);
    if (!index.isValid())
        return;

    mMainTreeView->cellClicked(index);
}
