#include "QtHelpers/ItemModelVirtualTable.h"
#include "Configuration.h"
#include "Data/Entity.h"
#include "Data/VirtualTableLookup.h"
#include "Spelunky2.h"
#include "pluginmain.h"

QVariant S2Plugin::ItemModelVirtualTable::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto& entry = Spelunky2::get()->get_VirtualTableLookup().entryForOffset(index.row());
        switch (index.column())
        {
            case gsColTableOffset:
                return entry.offset;
            case gsColCodeAddress:
                if (entry.isValidAddress)
                {
                    return QString::asprintf("<font color='green'><u>0x%016llX</u></font>", entry.value);
                }
                else
                {
                    return QString::asprintf("<span style='text-decoration: line-through'>0x%016llX</span>", entry.value);
                }
            case gsColTableAddress:
                return QString::asprintf("<font color='blue'><u>0x%016llX</u></font>", Spelunky2::get()->get_VirtualTableLookup().tableAddressForEntry(entry));
            case gsColSymbolName:
            {
                QStringList l;
                for (const auto& symbol : entry.symbols)
                {
                    l << QString::fromStdString(symbol);
                }
                return l.join(", ");
            }
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelVirtualTable::rowCount(const QModelIndex&) const
{
    return static_cast<int>(Spelunky2::get()->get_VirtualTableLookup().count());
}

QVariant S2Plugin::ItemModelVirtualTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColTableOffset:
                return "Table offset";
            case gsColCodeAddress:
                return "Code address";
            case gsColTableAddress:
                return "Table address";
            case gsColSymbolName:
                return "Symbol";
        }
    }
    return QVariant();
}

void S2Plugin::ItemModelVirtualTable::detectEntities()
{
    auto statePtr = Spelunky2::get()->get_StatePtr(false);
    if (statePtr == 0)
        return;

    if (mLayer0Offset == 0 || mLayer1Offset == 0)
    {
        auto config = Configuration::get();
        mLayer0Offset = config->offsetForField(config->typeFields(MemoryFieldType::State), "layer0", 0);
        mLayer1Offset = config->offsetForField(config->typeFields(MemoryFieldType::State), "layer1", 0);
    }

    auto processEntities = [&](size_t layerEntities, uint32_t count)
    {
        size_t maximum = (std::min)(count, 10000u);
        for (auto x = 0; x < maximum; ++x)
        {
            auto entityPtr = layerEntities + (x * sizeof(size_t));
            Entity entity{Script::Memory::ReadQword(entityPtr)};
            auto entityVTableOffset = Script::Memory::ReadQword(entity.ptr());

            auto entityName = entity.entityTypeName();
            Spelunky2::get()->get_VirtualTableLookup().setSymbolNameForOffsetAddress(entityVTableOffset, entityName);
        }
    };

    beginResetModel();
    auto layer0 = Script::Memory::ReadQword(mLayer0Offset + statePtr);
    auto layer0Count = Script::Memory::ReadDword(layer0 + 28);
    auto layer0Entities = Script::Memory::ReadQword(layer0 + 8);
    processEntities(layer0Entities, layer0Count);

    auto layer1 = Script::Memory::ReadQword(mLayer1Offset + statePtr);
    auto layer1Count = Script::Memory::ReadDword(layer1 + 28);
    auto layer1Entities = Script::Memory::ReadQword(layer1 + 8);
    processEntities(layer1Entities, layer1Count);
    endResetModel();
}

S2Plugin::SortFilterProxyModelVirtualTable::SortFilterProxyModelVirtualTable(QObject* parent) : QSortFilterProxyModel(parent) {}

bool S2Plugin::SortFilterProxyModelVirtualTable::filterAcceptsRow(int sourceRow, const QModelIndex&) const
{
    const auto& entry = Spelunky2::get()->get_VirtualTableLookup().entryForOffset(sourceRow);

    // only do text filtering when symbolless entries are not shown
    // because we will just jump to the first match in ViewVirtualTable::filterTextChanged
    if (!mShowSymbollessEntries && !mFilterString.isEmpty() && entry.symbols.size() > 0)
    {
        bool found = false;
        for (const auto& symbol : entry.symbols)
        {
            if (QString::fromStdString(symbol).contains(mFilterString, Qt::CaseInsensitive))
            {
                found = true;
            }
        }
        if (!found)
        {
            return false;
        }
    }

    if (!mShowImportedSymbols && entry.isAutoSymbol)
        return false;

    if (!mShowNonAddressEntries && !entry.isValidAddress)
        return false;

    if (!mShowSymbollessEntries && entry.symbols.size() == 0)
        return false;

    return true;
}
