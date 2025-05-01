#pragma once

#include <QSortFilterProxyModel>
#include <QString>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable : public QSortFilterProxyModel
    {
        Q_OBJECT
      public:
        using QSortFilterProxyModel::QSortFilterProxyModel;
        void setFilterString(const QString& f)
        {
            mFilterString = f;
            invalidateFilter();
        }

      protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

      private:
        QString mFilterString = "";
    };
} // namespace S2Plugin
