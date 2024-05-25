#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

namespace S2Plugin
{
    class Logger;

    class ItemModelLoggerFields : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        ItemModelLoggerFields(Logger* logger, QObject* parent = nullptr) : QAbstractItemModel(parent), mLogger(logger){};

        void removeRow(int index)
        {
            beginRemoveRows(QModelIndex(), index, index);
        }
        void removeRowEnd()
        {
            endRemoveRows();
        }
        void appendRow()
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount());
        }
        void appendRowEnd()
        {
            endInsertRows();
        }

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
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

      private:
        Logger* mLogger;
    };
} // namespace S2Plugin
