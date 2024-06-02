#include "Views/ViewStdVector.h"

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdVector::ViewStdVector(const std::string& vectorType, uintptr_t vectorAddr, QWidget* parent) : mVectorType(vectorType), mVectorAddress(vectorAddr), AbstractContainerView(parent)
{
    mVectorTypeSize = Configuration::get()->getTypeSize(mVectorType);

    setWindowTitle(QString("std::vector<%1>").arg(QString::fromStdString(vectorType)));
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment);
    reloadContainer();
}

void S2Plugin::ViewStdVector::reloadContainer()
{
    auto config = Configuration::get();
    mMainTreeView->clear();

    uintptr_t vectorBegin = Script::Memory::ReadQword(mVectorAddress);
    uintptr_t vectorEnd = Script::Memory::ReadQword(mVectorAddress + sizeof(uintptr_t));
    if (vectorBegin == vectorEnd)
        return;

    if (vectorBegin >= vectorEnd || !Script::Memory::IsValidPtr(vectorBegin) || !Script::Memory::IsValidPtr(vectorEnd))
        return; // TODO display error

    auto vectorItemCount = (vectorEnd - vectorBegin) / mVectorTypeSize;
    if ((vectorEnd - vectorBegin) % mVectorTypeSize != 0)
        return; // TODO display error

    // limit big vectors
    vectorItemCount = std::min(300ull, vectorItemCount);

    MemoryField field;
    if (config->isPermanentPointer(mVectorType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mVectorType;
        field.isPointer = true;
    }
    else if (config->isJsonStruct(mVectorType))
    {
        field.type = MemoryFieldType::DefaultStructType;
        field.jsonName = mVectorType;
    }
    else if (auto type = config->getBuiltInType(mVectorType); type != MemoryFieldType::None)
    {
        field.type = type;
        if (Configuration::isPointerType(type))
            field.isPointer = true;
    }
    else
    {
        dprintf("unknown type in ViewStdVector::refreshVectorContents() (%s)\n", mVectorType.c_str());
        return;
    }
    for (auto x = 0; x < vectorItemCount; ++x)
    {
        field.name = "obj_" + std::to_string(x);
        mMainTreeView->addMemoryField(field, field.name, vectorBegin + x * mVectorTypeSize, x * mVectorTypeSize);
    }
    mMainTreeView->updateTableHeader();
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 125);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    mMainTreeView->updateTree(0, 0, true);
}
