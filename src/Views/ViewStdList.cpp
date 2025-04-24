#include "Views/ViewStdList.h"

#include "Data/StdList.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "QtHelpers/WidgetPagination.h"
#include "pluginmain.h"
#include <QString>

S2Plugin::ViewStdList::ViewStdList(uintptr_t addr, const std::string& valueType, bool oldType, QWidget* parent) : AbstractContainerView(parent), mListAddress(addr), mOldType(oldType)
{
    mValueField = Configuration::get()->nameToMemoryField(valueType);
    setWindowTitle(QString("std::list<%1>").arg(QString::fromStdString(valueType)));
    mMainTreeView->activeColumns.disable(gsColComparisonValue).disable(gsColComparisonValueHex).disable(gsColComment).disable(gsColMemoryAddressDelta);
    mMainTreeView->updateTableHeader(false);
    mMainTreeView->setColumnWidth(gsColField, 145);
    mMainTreeView->setColumnWidth(gsColValueHex, 125);
    mMainTreeView->setColumnWidth(gsColMemoryAddress, 120);
    mMainTreeView->setColumnWidth(gsColType, 100);
    mMainTreeView->setColumnWidth(gsColValue, 300);
    reloadContainer();
}

void S2Plugin::ViewStdList::reloadContainer()
{
    mMainTreeView->clear();

    if (mValueField.type == MemoryFieldType::None)
    {
        mPagination->setSize(0);
        return;
    }

    if (mOldType)
    {
        OldStdList theList{mListAddress};
        if (!theList.isValid())
        {
            mPagination->setSize(0);
            return;
        }
        // had to make up some solution for pagination since the old list does not have an easy way to get the size
        size_t perPage = mPagination->recordsPerPage();
        size_t currentPageIndex = mPagination->getCurrentPage() - 1;
        size_t thisPageEnd = currentPageIndex * perPage + perPage;
        size_t x = 0;
        auto cur = theList.begin();

        for (; x < thisPageEnd && cur != theList.end(); ++x, ++cur)
        {
            if (x < currentPageIndex * perPage)
                continue;

            mValueField.name = "val_" + std::to_string(x);
            mMainTreeView->addMemoryField(mValueField, mValueField.name, cur.value_ptr(), 0);
        }
        if (cur != theList.end())
            mPagination->setSize(x + perPage - 1); // to add at least one more page
        else
            mPagination->setSize(x - 1);
    }
    else
    {
        StdList theList{mListAddress};
        if (!Script::Memory::IsValidPtr(theList.end().address()))
        {
            mPagination->setSize(0);
            return;
        }

        mPagination->setSize(theList.size());
        if (theList.size() == 0)
            return;

        auto range = mPagination->getRange();
        auto cur = theList.begin();
        auto end = theList.end();

        for (size_t x = 0; x < range.second && cur != end; ++x, ++cur)
        {
            if (x < range.first)
                continue;

            mValueField.name = "val_" + std::to_string(x);
            mMainTreeView->addMemoryField(mValueField, mValueField.name, cur.value_ptr(), 0);
        }
    }

    mMainTreeView->updateTableHeader();
    mMainTreeView->updateTree(0, 0, true);
}
