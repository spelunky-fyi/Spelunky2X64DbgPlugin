#include "Views/ViewStruct.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtHelpers/WidgetPagination.h"
#include "QtPlugin.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
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
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, mMainTreeView, static_cast<void (TreeViewMemoryFields::*)()>(&TreeViewMemoryFields::updateTree));
    refreshLayout->addWidget(autoRefresh);

    refreshLayout->addStretch();

    auto labelButton = new QPushButton("Label", this);
    QObject::connect(labelButton, &QPushButton::clicked, mMainTreeView, static_cast<void (TreeViewMemoryFields::*)()>(&TreeViewMemoryFields::labelAll));
    refreshLayout->addWidget(labelButton);

    mainLayout->addWidget(mMainTreeView);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColField, 200);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->addMemoryFields(fields, name, address);
    mMainTreeView->updateTree(0, 0, true);
    autoRefresh->toggleAutoRefresh(true);
}

S2Plugin::ViewArray::ViewArray(uintptr_t address, std::string arrayTypeName, size_t num, std::string name, QWidget* parent)
    : ViewStruct(0, {}, arrayTypeName + " " + name + '[' + std::to_string(num) + ']', parent), mArrayAddress(address)
{

    mArray.name = name;
    mArray.type = MemoryFieldType::Array;
    mArray.firstParameterType = arrayTypeName;
    mArray.secondParameterType = '#'; // just to let it know it should put all the elements in, no array element
    mArray.numberOfElements = num;

    mPagination = new WidgetPagination(this);
    layout()->addWidget(mPagination);
    QObject::connect(mPagination, &WidgetPagination::pageUpdate, this, &ViewArray::pageListUpdate);
    mPagination->setSize(num);
    pageListUpdate();
}

void S2Plugin::ViewArray::pageListUpdate()
{
    mMainTreeView->clear();
    mMainTreeView->updateTableHeader();
    auto range = mPagination->getRange();
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

    mPagination = new WidgetPagination(this);
    layout()->addWidget(mPagination);
    QObject::connect(mPagination, &WidgetPagination::pageUpdate, this, &ViewMatrix::pageListUpdate);
    mPagination->setSize(rows);
    pageListUpdate();
}

void S2Plugin::ViewMatrix::pageListUpdate()
{
    mMainTreeView->clear();
    mMainTreeView->updateTableHeader();
    auto range = mPagination->getRange();
    // dirty way to transfer the first index
    mMatrix.secondParameterType = std::to_string(range.first);
    mMatrix.rows = range.second;
    mMainTreeView->addMemoryField(mMatrix, {}, mMatrixAddress, 0);
}

QSize S2Plugin::ViewStruct::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewStruct::minimumSizeHint() const
{
    return QSize(150, 150);
}
