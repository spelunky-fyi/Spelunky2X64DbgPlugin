#include "QtHelpers/ItemModelLoggerFields.h"
#include "Configuration.h"
#include "Data/Logger.h"
#include "QtHelpers/TableViewLogger.h"

QVariant S2Plugin::ItemModelLoggerFields::data(const QModelIndex& index, int role) const
{
    auto& field = mLogger->fieldAt(index.row());
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case gsLogFieldColColor:
            {
                return field.color;
            }
            case gsLogFieldColMemoryOffset:
            {
                return QString::asprintf("0x%016llX", field.memoryAddr);
            }
            case gsLogFieldColFieldName:
            {
                return QString::fromStdString(field.name);
            }
            case gsLogFieldColFieldType:
            {
                auto str = Configuration::getTypeDisplayName(field.type);
                return QString::fromUtf8(str.data(), str.size());
            }
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelLoggerFields::rowCount(const QModelIndex& parent) const
{
    return mLogger->fieldCount();
}

QVariant S2Plugin::ItemModelLoggerFields::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsLogFieldColColor:
                return "Color";
            case gsLogFieldColMemoryOffset:
                return "Memory offset";
            case gsLogFieldColFieldType:
                return "Type";
            case gsLogFieldColFieldName:
                return "Name";
        }
    }
    return QVariant();
}
