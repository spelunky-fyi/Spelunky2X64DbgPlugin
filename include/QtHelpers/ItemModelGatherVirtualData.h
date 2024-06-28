#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QVariant>
#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    static const uint8_t gsColGatherID = 0;
    static const uint8_t gsColGatherName = 1;
    static const uint8_t gsColGatherVirtualTableOffset = 2;
    static const uint8_t gsColGatherCollision1Present = 3;
    static const uint8_t gsColGatherCollision2Present = 4;
    static const uint8_t gsColGatherOpenPresent = 5;
    static const uint8_t gsColGatherDamagePresent = 6;
    static const uint8_t gsColGatherKillPresent = 7;
    static const uint8_t gsColGatherDestroyPresent = 8;
    static const uint8_t gsColGatherStatemachinePresent = 9;

    struct GatheredDataEntry
    {
        size_t id;
        QString name;
        size_t virtualTableOffset;
        bool collision1Present;
        bool collision2Present;
        bool openPresent;
        bool damagePresent;
        bool killPresent;
        bool destroyPresent;
        bool statemachinePresent;
    };

    class ItemModelGatherVirtualData : public QAbstractItemModel
    {
        Q_OBJECT

      public:
        explicit ItemModelGatherVirtualData(QObject* parent = nullptr) : QAbstractItemModel(parent)
        {
            parseJSON();
        };

        Qt::ItemFlags flags(const QModelIndex&) const override
        {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
        }
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return static_cast<int>(mEntries.size());
        }
        int columnCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return 10;
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

        void gatherEntities();
        void gatherExtraObjects();
        void gatherAvailableVirtuals();
        float completionPercentage() const;
        std::string dumpJSON() const;
        std::string dumpVirtTable() const;
        std::string dumpCppEnum() const;
        bool isEntryCompleted(size_t index) const
        {
            return mEntries.at(index).virtualTableOffset != 0;
        }

      private:
        std::vector<GatheredDataEntry> mEntries;

        void parseJSON();
    };

    class SortFilterProxyModelGatherVirtualData : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        explicit SortFilterProxyModelGatherVirtualData(QObject* parent = nullptr) : QSortFilterProxyModel(parent){};

        bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override
        {
            if (mHideCompleted && dynamic_cast<ItemModelGatherVirtualData*>(sourceModel())->isEntryCompleted(sourceRow))
            {
                return false;
            }
            return true;
        }
        void setHideCompleted(bool b)
        {
            beginResetModel();
            mHideCompleted = b;
            endResetModel();
        }

      private:
        bool mHideCompleted = false;
    };

} // namespace S2Plugin
