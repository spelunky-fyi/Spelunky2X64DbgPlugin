#include "Views/ViewEntityList.h"

#include "Configuration.h"
#include "Data/EntityList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include <QString>

S2Plugin::ViewEntityList::ViewEntityList(uintptr_t address, QWidget* parent) : mEntityListAddress(address), AbstractContainerView(parent)
{
    setWindowTitle("EntityList");
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment); //.disable(gsColMemoryAddressDelta).disable(gsColMemoryAddress);
    reloadContainer();
}

void S2Plugin::ViewEntityList::reloadContainer()
{
    mMainTreeView->clear();

    EntityList entityList{mEntityListAddress};

    auto entities = entityList.entities();
    auto uids = entityList.getAllUids();
    for (size_t idx = 0; idx < entityList.size(); ++idx)
    {
        MemoryField entityField;
        entityField.name = "uid_" + std::to_string(uids[idx]);
        entityField.isPointer = true;
        entityField.type = MemoryFieldType::EntityPointer;
        mMainTreeView->addMemoryField(entityField, {}, entities + idx * sizeof(uintptr_t), idx * sizeof(uintptr_t));
    }

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree(0, 0, true);
}
