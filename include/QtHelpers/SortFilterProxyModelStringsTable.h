#pragma once

#include <QSortFilterProxyModel>
#include <QString>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelStringsTable(QObject* parent = nullptr) : QSortFilterProxyModel(parent){};

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
