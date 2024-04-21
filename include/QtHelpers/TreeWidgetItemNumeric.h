#pragma once

#include <QTreeWidgetItem>

namespace S2Plugin
{
    class TreeWidgetItemNumeric : public QTreeWidgetItem
    {
      public:
        using QTreeWidgetItem::QTreeWidgetItem;
        bool operator<(const QTreeWidgetItem& other) const
        {
            return data(0, Qt::UserRole) < other.data(0, Qt::UserRole);
        }
    };

} // namespace S2Plugin
