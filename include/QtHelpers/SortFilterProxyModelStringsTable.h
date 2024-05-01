#pragma once

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QString>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        using QSortFilterProxyModel::QSortFilterProxyModel;

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setFilterString(const QString& f)
        {
            mFilterString = f;
            invalidateFilter();
        }

      private:
        QString mFilterString = "";
    };

} // namespace S2Plugin
