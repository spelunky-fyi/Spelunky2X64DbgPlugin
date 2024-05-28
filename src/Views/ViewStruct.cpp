#include "Views/ViewStruct.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

S2Plugin::ViewStruct::ViewStruct(uintptr_t address, const std::vector<MemoryField>& fields, const std::string name, QWidget* parent) : QWidget(parent)
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

S2Plugin::ViewArray::ViewArray(uintptr_t address, MemoryField field, size_t num, const std::string name, QWidget* parent) : ViewStruct(address, {}, name, parent)
{
    size_t currentAddr = address;
    size_t currentDelta = 0;
    size_t size = field.get_size();
    field.name = "index_000";

    for (size_t idx = 0; idx < num; ++idx)
    {
        auto indexString = std::to_string(idx);
        auto itr = 9 - indexString.length(); // 9 - lenght of "index_000"
        field.name.replace(itr, indexString.length(), indexString);

        mMainTreeView->addMemoryField(field, name + "." + field.name, currentAddr, currentDelta, 0, nullptr);
        currentDelta += size;
        if (address != 0)
            currentAddr += size;
    }
}

QSize S2Plugin::ViewStruct::sizeHint() const
{
    return QSize(750, 1050);
}

QSize S2Plugin::ViewStruct::minimumSizeHint() const
{
    return QSize(150, 150);
}
