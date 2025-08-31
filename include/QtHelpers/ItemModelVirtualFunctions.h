#pragma once

#include "Configuration.h" // for VirtualFunction
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace S2Plugin
{
    constexpr const uint16_t gsRoleFunctionIndex = Qt::UserRole;
    constexpr const uint16_t gsRoleFunctionTableAddress = Qt::UserRole + 1;
    constexpr const uint16_t gsRoleFunctionFunctionAddress = Qt::UserRole + 2;

    class ItemModelVirtualFunctions : public QAbstractItemModel
    {
        Q_OBJECT
      public:
        explicit ItemModelVirtualFunctions(std::string_view typeName, uintptr_t memoryAddress, QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex&) const override
        {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
        }
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return static_cast<int>(mFunctions.size());
        }
        int columnCount([[maybe_unused]] const QModelIndex& parent = QModelIndex()) const override
        {
            return 6;
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

        enum Column
        {
            Index = 0,
            Offset = 1,
            TableAddress = 2,
            FunctionAddress = 3,
            Signature = 4,
            Comment = 5,
        };

      private:
        std::string mTypeName;
        uintptr_t mMemoryAddress;
        std::vector<VirtualFunction> mFunctions;
    };
} // namespace S2Plugin
