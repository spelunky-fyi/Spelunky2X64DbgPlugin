#include "Views/ViewEntityList.h"

#include "Data/EntityList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetPagination.h"
#include <QString>

S2Plugin::ViewEntityList::ViewEntityList(uintptr_t address, QWidget* parent) : AbstractContainerView(parent), mEntityListAddress(address)
{
    setWindowTitle("EntityList");
    mEntityField.isPointer = true;
    mEntityField.type = MemoryFieldType::EntityPointer;

    mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColField, 120);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 200);
    reloadContainer();
}

void S2Plugin::ViewEntityList::reloadContainer()
{
    mMainTreeView->clear();

    EntityList entityList{mEntityListAddress};

    mPagination->setSize(entityList.size());
    if (entityList.size() == 0)
        return;

    auto entities = entityList.entities();
    auto uids = entityList.getAllUids();
    auto range = mPagination->getRange();
    for (size_t idx = range.first; idx < range.second; ++idx)
    {
        mEntityField.name = "uid_" + std::to_string(uids[idx]);
        mMainTreeView->addMemoryField(mEntityField, {}, entities + idx * sizeof(uintptr_t), idx * sizeof(uintptr_t));
    }

    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
}
