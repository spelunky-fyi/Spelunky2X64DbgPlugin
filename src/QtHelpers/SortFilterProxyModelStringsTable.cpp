#include "QtHelpers/SortFilterProxyModelStringsTable.h"

#include "Views/ViewStringsTable.h" // just for gsColStringValue

bool S2Plugin::SortFilterProxyModelStringsTable::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
    if (mFilterString.isEmpty())
    {
        return true;
    }
    QAbstractItemModel* model = sourceModel();

    if (sourceRow < 0 || sourceRow >= model->rowCount())
    {
        return false;
    }

    bool isNumeric = false;
    auto enteredID = mFilterString.toInt(&isNumeric);
    if (isNumeric)
    {
        return enteredID == sourceRow;
    }
    else
    {
        auto str = model->data(model->index(sourceRow, gsColStringValue), Qt::DisplayRole).toString();
        return str.contains(mFilterString, Qt::CaseInsensitive);
    }
}
