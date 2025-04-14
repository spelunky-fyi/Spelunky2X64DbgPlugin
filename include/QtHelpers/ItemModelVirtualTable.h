#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVariant>
#include <cstdint>

namespace S2Plugin
{
    static const uint8_t gsColTableOffset = 0;
    static const uint8_t gsColCodeAddress = 1;
    static const uint8_t gsColTableAddress = 2;
    static const uint8_t gsColSymbolName = 3;

    class ItemModelVirtualTable : public QAbstractItemModel
    {
        Q_OBJECT

      public:
        explicit ItemModelVirtualTable(QObject* parent = nullptr) : QAbstractItemModel(parent){};

        Qt::ItemFlags flags(const QModelIndex&) const override
        {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
        }
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return 4;
        }
        QModelIndex index(int row, int column, [[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return createIndex(row, column);
        }
        QModelIndex parent(const QModelIndex&) const override
        {
            return QModelIndex();
        }
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        void detectEntities();

      private:
        uintptr_t mLayer0Offset;
        uintptr_t mLayer1Offset;
    };

    class SortFilterProxyModelVirtualTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        explicit SortFilterProxyModelVirtualTable(QObject* parent = nullptr);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setShowImportedSymbols(bool b)
        {
            mShowImportedSymbols = b;
            invalidateFilter();
        }
        void setShowNonAddressEntries(bool b)
        {
            mShowNonAddressEntries = b;
            invalidateFilter();
        }
        void setShowSymbollessEntries(bool b)
        {
            mShowSymbollessEntries = b;
            invalidateFilter();
        }
        void setFilterString(const QString& f)
        {
            mFilterString = f;
            invalidateFilter();
        }

        bool symbollessEntriesShown() const noexcept
        {
            return mShowSymbollessEntries;
        }

      private:
        bool mShowImportedSymbols = true;
        bool mShowNonAddressEntries = true;
        bool mShowSymbollessEntries = true;
        QString mFilterString = "";
    };

} // namespace S2Plugin
