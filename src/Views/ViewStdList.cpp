#include "Views/ViewStdList.h"

#include "Configuration.h"
#include "Data/OldStdList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdList::ViewStdList(uintptr_t addr, const std::string& valueType, bool oldType, QWidget* parent)
    : mListType(valueType), mListAddress(addr), mOldType(oldType), AbstractContainerView(parent)
{
    setWindowTitle(QString("std::list<%1>").arg(QString::fromStdString(mListType)));
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment).disable(gsColMemoryAddressDelta);
    reloadContainer();
}

void S2Plugin::ViewStdList::reloadContainer()
{
    auto config = Configuration::get();
    mMainTreeView->clear();

    MemoryField field;
    if (config->isPermanentPointer(mListType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mListType;
        field.isPointer = true;
    }
    else if (config->isJsonStruct(mListType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mListType;
    }
    else if (auto type = config->getBuiltInType(mListType); type != MemoryFieldType::None)
    {
        field.type = type;
        if (Configuration::isPointerType(type))
            field.isPointer = true;
    }
    else
    {
        dprintf("unknown type in ViewStdList::refreshVectorContents() (%s)\n", mListType.c_str());
        return;
    }
    if (mOldType)
    {
        OldStdList theList{mListAddress};
        size_t x = 0;
        for (auto val_addr : theList)
        {
            field.name = "val_" + std::to_string(x++);
            mMainTreeView->addMemoryField(field, field.name, val_addr, 0);
        }
    }
    else
    {
        // TODO new type implement, if found
    }

    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree(0, 0, true);
}
