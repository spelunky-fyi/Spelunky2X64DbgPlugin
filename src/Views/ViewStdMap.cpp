#include "Views/ViewStdMap.h"

#include "Configuration.h"
#include "Data/StdMap.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdMap::ViewStdMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName, QWidget* parent)
    : mMapKeyType(keytypeName), mMapValueType(valuetypeName), mMapAddress(address), AbstractContainerView(parent)
{
    auto config = Configuration::get();
    mMapKeyTypeSize = config->getTypeSize(keytypeName);
    mMapValueTypeSize = config->getTypeSize(valuetypeName);

    mMapKeyAlignment = config->getAlingment(keytypeName);
    mMapValueAlignment = mMapValueTypeSize == 0 ? 0 : config->getAlingment(valuetypeName);

    if (mMapValueTypeSize == 0)
        setWindowTitle(QString("std::set<%1>").arg(QString::fromStdString(keytypeName)));
    else
        setWindowTitle(QString("std::map<%1, %2>").arg(QString::fromStdString(keytypeName), QString::fromStdString(valuetypeName)));

    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColMemoryAddressDelta).disable(gsColComment);
    reloadContainer();
}

void S2Plugin::ViewStdMap::reloadContainer()
{
    if (!Script::Memory::IsValidPtr(Script::Memory::ReadQword(mMapAddress)))
        return;

    StdMap the_map{mMapAddress, mMapKeyAlignment, mMapValueAlignment, mMapKeyTypeSize};
    auto config = Configuration::get();
    mMainTreeView->clear();

    MemoryField key_field = config->nameToMemoryField(mMapKeyType);
    key_field.name = "key";
    if (key_field.type == MemoryFieldType::None)
        return;

    MemoryField value_field;
    if (mMapValueTypeSize != 0) // if not StdSet
    {
        value_field = config->nameToMemoryField(mMapValueType);
        value_field.name = "value";
        if (value_field.type == MemoryFieldType::None)
            return;
    }

    auto _end = the_map.end();
    auto _cur = the_map.begin();
    MemoryField parent_field;
    parent_field.type = MemoryFieldType::Dummy;
    for (int x = 0; _cur != _end && x < 300; ++x, ++_cur)
    {
        if (mMapValueTypeSize == 0) // StdSet
        {
            key_field.name = "key_" + std::to_string(x);
            mMainTreeView->addMemoryField(key_field, key_field.name, _cur.key_ptr(), 0);
        }
        else // StdMap
        {
            parent_field.name = "obj_" + std::to_string(x);
            QStandardItem* parent = mMainTreeView->addMemoryField(parent_field, parent_field.name, 0, 0);
            mMainTreeView->addMemoryField(key_field, key_field.name, _cur.key_ptr(), 0, 0, parent);
            mMainTreeView->addMemoryField(value_field, value_field.name, _cur.value_ptr(), 0, 0, parent);
        }
    }
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree(0, 0, true);
}
