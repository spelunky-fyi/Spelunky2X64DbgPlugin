#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    static const uint8_t gsColFunctionIndex = 0;
    static const uint8_t gsColFunctionTableAddress = 1;
    static const uint8_t gsColFunctionFunctionAddress = 2;
    static const uint8_t gsColFunctionSignature = 3;

    static const uint16_t gsRoleFunctionIndex = Qt::UserRole;
    static const uint16_t gsRoleFunctionTableAddress = Qt::UserRole + 1;
    static const uint16_t gsRoleFunctionFunctionAddress = Qt::UserRole + 2;

    class ItemModelVirtualFunctions : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        explicit ItemModelVirtualFunctions(const std::string& typeName, uintptr_t memoryAddress, QObject* parent = nullptr)
            : QAbstractItemModel(parent), mTypeName(typeName), mMemoryAddress(memoryAddress){};

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

      private:
        std::string mTypeName;
        uintptr_t mMemoryAddress;
    };

    class SortFilterProxyModelVirtualFunctions : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        explicit SortFilterProxyModelVirtualFunctions(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
        {
            setSortRole(gsRoleFunctionIndex);
        }

      protected:
        bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
        {
            auto leftValue = sourceModel()->data(source_left, gsRoleFunctionIndex).toLongLong();
            auto rightValue = sourceModel()->data(source_right, gsRoleFunctionIndex).toLongLong();
            return leftValue < rightValue;
        }
    };
} // namespace S2Plugin
