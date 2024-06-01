#include "Views/ViewEntityList.h"

#include "Configuration.h"
#include "Data/EntityList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetAutorefresh.h"
#include "QtPlugin.h"
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

S2Plugin::ViewEntityList::ViewEntityList(uintptr_t address, QWidget* parent) : mEntityListAddress(address), QWidget(parent)
{
    setWindowIcon(getCavemanIcon());
    setWindowTitle("EntityList");

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(5);
    auto refreshLayout = new QHBoxLayout();
    mainLayout->addLayout(refreshLayout);

    auto refreshVectorButton = new QPushButton("Refresh list", this);
    QObject::connect(refreshVectorButton, &QPushButton::clicked, this, &ViewEntityList::refreshEntityListContents);
    refreshLayout->addWidget(refreshVectorButton);

    auto autoRefresh = new WidgetAutorefresh(300, this);
    refreshLayout->addWidget(autoRefresh);

    mMainTreeView = new TreeViewMemoryFields(this);
    QObject::connect(autoRefresh, &WidgetAutorefresh::refresh, mMainTreeView, static_cast<void (TreeViewMemoryFields::*)()>(&TreeViewMemoryFields::updateTree));
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment).disable(gsColMemoryAddressDelta).disable(gsColMemoryAddress);
    mainLayout->addWidget(mMainTreeView);
    autoRefresh->toggleAutoRefresh(true);
    refreshEntityListContents();
}

void S2Plugin::ViewEntityList::refreshEntityListContents()
{
    mMainTreeView->clear();

    EntityList entityList{mEntityListAddress};

    // TODO using custom view instead of memory view would improve performance
    auto entities = entityList.entities();
    auto uids = entityList.getAllUids();
    for (size_t idx = 0; idx < entityList.size(); ++idx)
    {
        MemoryField entityField;
        entityField.name = "uid_" + std::to_string(uids[idx]);
        entityField.isPointer = true;
        entityField.type = MemoryFieldType::EntityPointer;
        mMainTreeView->addMemoryField(entityField, {}, entities + idx * sizeof(uintptr_t), 0);
    }

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree(0, 0, true);
}

QSize S2Plugin::ViewEntityList::sizeHint() const
{
    return QSize(750, 550);
}

QSize S2Plugin::ViewEntityList::minimumSizeHint() const
{
    return QSize(150, 150);
}
