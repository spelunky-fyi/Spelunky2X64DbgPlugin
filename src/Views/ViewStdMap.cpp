#include "Views/ViewStdMap.h"

#include "Data/StdMap.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetPagination.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdMap::ViewStdMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName, QWidget* parent) : AbstractContainerView(parent), mMapAddress(address)
{
    auto config = Configuration::get();

    mKeyField = config->nameToMemoryField(keytypeName);
    mValueField = config->nameToMemoryField(valuetypeName);

    mMapKeyAlignment = config->getAlingment(keytypeName);
    mMapValueAlignment = valuetypeName.empty() ? 0u : config->getAlingment(valuetypeName);

    if (valuetypeName.empty())
        setWindowTitle(QString("std::set<%1>").arg(QString::fromStdString(keytypeName)));
    else
        setWindowTitle(QString("std::map<%1, %2>").arg(QString::fromStdString(keytypeName), QString::fromStdString(valuetypeName)));

    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColMemoryAddressDelta).disable(gsColComment);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setEnableTopBranchDrawing(false);
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 250);
    QObject::connect(mMainTreeView, &TreeViewMemoryFields::collapsed, this, &ViewStdMap::onItemCollapsed);
    reloadContainer();
}

void S2Plugin::ViewStdMap::reloadContainer()
{
    mMainTreeView->clear();
    if (mKeyField.type == MemoryFieldType::None)
    {
        mPagination->setSize(0);
        return;
    }
    if (!Script::Memory::IsValidPtr(Script::Memory::ReadQword(mMapAddress)))
    {
        mPagination->setSize(0);
        return;
    }

    StdMap the_map{mMapAddress, mMapKeyAlignment, mMapValueAlignment, mKeyField.get_size()};
    mPagination->setSize(the_map.size());

    if (the_map.size() == 0)
        return;

    auto range = mPagination->getRange();

    mKeyField.name = "key";
    if (mKeyField.type == MemoryFieldType::None)
        return;

    if (mValueField.type != MemoryFieldType::None) // if not StdSet
        mValueField.name = "value";

    auto _end = the_map.end();
    auto _cur = the_map.begin();
    MemoryField parent_field;
    parent_field.type = MemoryFieldType::Dummy;
    for (size_t x = 0; _cur != _end && x < range.second; ++x, ++_cur)
    {
        if (x < range.first)
            continue;

        if (mValueField.type == MemoryFieldType::None) // StdSet
        {
            mKeyField.name = "key_" + std::to_string(x);
            mMainTreeView->addMemoryField(mKeyField, mKeyField.name, _cur.key_ptr(), 0);
        }
        else // StdMap
        {
            parent_field.name = "obj_" + std::to_string(x);
            QStandardItem* parent = mMainTreeView->addMemoryField(parent_field, parent_field.name, 0, 0);
            mMainTreeView->addMemoryField(mKeyField, mKeyField.name, _cur.key_ptr(), 0, 0, parent);
            mMainTreeView->addMemoryField(mValueField, mValueField.name, _cur.value_ptr(), 0, 0, parent);
            mMainTreeView->setExpanded(parent->index(), true);
        }
    }
    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
}

void S2Plugin::ViewStdMap::onItemCollapsed(const QModelIndex& index)
{
    if (index.parent() == QModelIndex())
    {
        // prevent it from being collapsed
        mMainTreeView->setExpanded(index, true);
    }
}
