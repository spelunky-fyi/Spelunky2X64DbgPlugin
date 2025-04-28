#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace S2Plugin
{
    class ItemModelStates : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        explicit ItemModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent = nullptr) : QAbstractItemModel(parent), mStates(states){};

        Qt::ItemFlags flags(const QModelIndex&) const override
        {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
        }
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
        {
            auto& [stateID, state] = mStates.at(static_cast<size_t>(index.row()));
            if (role == Qt::DisplayRole)
            {
                return QString("%1: %2").arg(stateID).arg(QString::fromStdString(state));
            }
            else if (role == Qt::UserRole)
            {
                return stateID;
            }
            return QVariant();
        }
        int rowCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return static_cast<int>(mStates.size());
        }
        int columnCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return 1;
        }
        QModelIndex index(int row, int column, [[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return createIndex(row, column);
        }
        QModelIndex parent(const QModelIndex&) const override
        {
            return QModelIndex();
        }

      private:
        std::vector<std::pair<int64_t, std::string>> mStates;
    };

    class SortFilterProxyModelStates : public QSortFilterProxyModel
    {
        Q_OBJECT
      public:
        explicit SortFilterProxyModelStates(const std::vector<std::pair<int64_t, std::string>>& states, QObject* parent = nullptr) : QSortFilterProxyModel(parent), mStates(states)
        {
            setSortRole(Qt::UserRole);
        }

      protected:
        bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
        {
            auto leftValue = sourceModel()->data(source_left, Qt::UserRole).toLongLong();
            auto rightValue = sourceModel()->data(source_right, Qt::UserRole).toLongLong();
            return leftValue < rightValue;
        }

      private:
        std::vector<std::pair<int64_t, std::string>> mStates;
    };
} // namespace S2Plugin
