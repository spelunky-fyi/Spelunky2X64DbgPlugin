#include "Views/ViewStdVector.h"

#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetPagination.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdVector::ViewStdVector(uintptr_t address, const std::string& valueTypeName, QWidget* parent) : AbstractContainerView(parent), mVectorAddress(address)
{
    mValueField = Configuration::get()->nameToMemoryField(valueTypeName);

    setWindowTitle(QString("std::vector<%1>").arg(QString::fromStdString(valueTypeName)));
    mMainTreeView->mActiveColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    reloadContainer();
}

void S2Plugin::ViewStdVector::reloadContainer()
{
    mMainTreeView->clear();
    if (mValueField.type == MemoryFieldType::None)
    {
        mPagination->setSize(0);
        return; // TODO display error
    }

    uintptr_t vectorBegin = Script::Memory::ReadQword(mVectorAddress);
    uintptr_t vectorEnd = Script::Memory::ReadQword(mVectorAddress + sizeof(uintptr_t));
    if (vectorBegin == vectorEnd)
    {
        mPagination->setSize(0);
        return;
    }

    if (vectorBegin >= vectorEnd || !Script::Memory::IsValidPtr(vectorBegin) || !Script::Memory::IsValidPtr(vectorEnd))
    {
        mPagination->setSize(0);
        return; // TODO display error
    }
    auto vectorItemCount = (vectorEnd - vectorBegin) / mValueField.get_size();
    if ((vectorEnd - vectorBegin) % mValueField.get_size() != 0)
    {
        mPagination->setSize(0);
        return; // TODO display error
    }
    mPagination->setSize(vectorItemCount);
    if (vectorItemCount == 0)
        return;

    auto range = mPagination->getRange();

    for (auto x = range.first; x < range.second; ++x)
    {
        mValueField.name = "obj_" + std::to_string(x);
        mMainTreeView->addMemoryField(mValueField, mValueField.name, vectorBegin + x * mValueField.get_size(), x * mValueField.get_size());
    }
    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
}
