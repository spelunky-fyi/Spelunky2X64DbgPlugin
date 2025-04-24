#include "QtHelpers/ItemModelLoggerSamples.h"
#include "Configuration.h"
#include "Data/Logger.h"
#include "QtHelpers/TableViewLogger.h"

QVariant S2Plugin::ItemModelLoggerSamples::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return index.row();
        }
        else
        {
            const auto& field = mLogger->fieldAt(static_cast<size_t>(index.column()) - 1);
            const auto& samples = mLogger->samplesForField(field.uuid);
            size_t rowIndex = static_cast<size_t>(index.row());
            switch (field.type)
            {
                case MemoryFieldType::Byte:
                {
                    return std::any_cast<int8_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::UnsignedByte:
                case MemoryFieldType::Bool:
                case MemoryFieldType::Flags8:
                case MemoryFieldType::State8:
                case MemoryFieldType::CharacterDBID:
                {
                    return std::any_cast<uint8_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::Word:
                {
                    return std::any_cast<int16_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::UnsignedWord:
                case MemoryFieldType::Flags16:
                case MemoryFieldType::State16:
                {
                    return std::any_cast<uint16_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::Dword:
                case MemoryFieldType::TextureDBID:
                case MemoryFieldType::State32:
                case MemoryFieldType::EntityUID:
                {
                    return std::any_cast<int32_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::UnsignedDword:
                case MemoryFieldType::Flags32:
                case MemoryFieldType::EntityDBID:
                case MemoryFieldType::ParticleDBID:
                case MemoryFieldType::StringsTableID:
                {
                    return std::any_cast<uint32_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::Float:
                {
                    return std::any_cast<float>(samples.at(rowIndex));
                }
                case MemoryFieldType::Double:
                {
                    return std::any_cast<double>(samples.at(rowIndex));
                }
                case MemoryFieldType::Qword:
                {
                    return std::any_cast<int64_t>(samples.at(rowIndex));
                }
                case MemoryFieldType::UnsignedQword:
                {
                    return std::any_cast<uint64_t>(samples.at(rowIndex));
                }
            }
        }
    }
    return QVariant();
}

int S2Plugin::ItemModelLoggerSamples::rowCount(const QModelIndex&) const
{
    return static_cast<int>(mLogger->sampleCount());
}

int S2Plugin::ItemModelLoggerSamples::columnCount(const QModelIndex&) const
{
    return static_cast<int>(mLogger->fieldCount() + 1);
}

QVariant S2Plugin::ItemModelLoggerSamples::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case 0:
                return "Sample";
            default:
                return QString::fromStdString(mLogger->fieldAt(static_cast<size_t>(section) - 1).name);
        }
    }
    return QVariant();
}
