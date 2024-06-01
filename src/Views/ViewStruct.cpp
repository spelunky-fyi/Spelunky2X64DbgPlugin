#include "Views/ViewStruct.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
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
    mMainTreeView->addMemoryFields(fields, name, address);
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    mMainTreeView->setColumnWidth(gsColField, 125);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddressDelta, 75);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
    autoRefresh->toggleAutoRefresh(true);
}

S2Plugin::ViewArray::ViewArray(uintptr_t address, std::string arrayTypeName, size_t num, std::string name, QWidget* parent)
    : ViewStruct(0, {}, arrayTypeName + " " + name + '[' + std::to_string(num) + ']', parent)
{
    MemoryField array;
    array.name = name;
    array.type = MemoryFieldType::Array;
    array.firstParameterType = arrayTypeName;
    array.secondParameterType = '#'; // just to let it know it should put all the elements in, no array element
    array.numberOfElements = num;
    mMainTreeView->addMemoryField(array, {}, address, 0);
}

S2Plugin::ViewMatrix::ViewMatrix(uintptr_t address, std::string arrayTypeName, size_t rows, size_t columns, std::string name, QWidget* parent)
    : ViewStruct(0, {}, arrayTypeName + " " + name + '[' + std::to_string(rows) + "][" + std::to_string(columns) + ']', parent)
{
    MemoryField matrix;
    matrix.name = name;
    matrix.type = MemoryFieldType::Matrix;
    matrix.firstParameterType = arrayTypeName;
    matrix.secondParameterType = '$'; // just to let it know it should put all the elements in, no matrix element
                                      // it can't be # since we still want the array to be placed normally, just no size limit
    matrix.rows = rows;
    matrix.columns = columns;
    mMainTreeView->addMemoryField(matrix, {}, address, 0);
}

QSize S2Plugin::ViewStruct::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewStruct::minimumSizeHint() const
{
    return QSize(150, 150);
}
