#pragma once

#include <QTableWidgetItem>

namespace S2Plugin
{
    class TableWidgetItemNumeric : public QTableWidgetItem
    {
      public:
        using QTableWidgetItem::QTableWidgetItem;
        bool operator<(const QTableWidgetItem& other) const
        {
            return data(Qt::UserRole) < other.data(Qt::UserRole);
        }
    };
} // namespace S2Plugin
