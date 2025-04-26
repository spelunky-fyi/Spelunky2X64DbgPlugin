#include "Data/Logger.h"
#include "Configuration.h"
#include "QtHelpers/ItemModelLoggerFields.h"
#include "pluginmain.h"
#include "read_helpers.h"

void S2Plugin::Logger::addField(const LoggerField& field)
{
    if (mTableModel != nullptr)
    {
        mTableModel->appendRow();
        mFields.emplace_back(field);
        mTableModel->appendRowEnd();
        mSamples.clear();
        emit fieldsChanged();
    }
}

void S2Plugin::Logger::removeFieldAt(int fieldIndex)
{
    if (mTableModel != nullptr)
    {
        mTableModel->removeRow(fieldIndex);
        mFields.erase(mFields.begin() + fieldIndex);
        mTableModel->removeRowEnd();
        mSamples.clear();
        emit fieldsChanged();
    }
}

void S2Plugin::Logger::start(int samplePeriod, int duration)
{
    mSamples.clear();

    mSampleTimer = std::make_unique<QTimer>(this);
    mDurationTimer = std::make_unique<QTimer>(this);
    QObject::connect(mSampleTimer.get(), &QTimer::timeout, this, &Logger::sample);
    QObject::connect(mDurationTimer.get(), &QTimer::timeout, this, &Logger::durationEnded);

    mSampleTimer->setTimerType(Qt::PreciseTimer);
    mSampleTimer->setInterval(samplePeriod);

    mDurationTimer->setTimerType(Qt::PreciseTimer);
    mDurationTimer->setSingleShot(true);
    mDurationTimer->setInterval(duration * 1000);

    for (const auto& field : mFields)
    {
        std::vector<std::any> v;
        mSamples[field.uuid] = v;
    }

    mDurationTimer->start();
    mSampleTimer->start();
}

void S2Plugin::Logger::sample()
{
    for (const auto& field : mFields)
    {
        switch (field.type)
        {
            case MemoryFieldType::Byte:
            case MemoryFieldType::State8:
            {
                int8_t value = Read<int8_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::UnsignedByte:
            case MemoryFieldType::Bool:
            case MemoryFieldType::Flags8:
            case MemoryFieldType::CharacterDBID:
            {
                uint8_t value = Read<uint8_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::Word:
            case MemoryFieldType::State16:
            {
                int16_t value = Read<int16_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::UnsignedWord:
            case MemoryFieldType::Flags16:
            {
                uint16_t value = Read<uint16_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::Dword:
            case MemoryFieldType::State32:
            case MemoryFieldType::EntityUID:
            case MemoryFieldType::TextureDBID:
            {
                int32_t value = Read<int32_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::UnsignedDword:
            case MemoryFieldType::Flags32:
            case MemoryFieldType::EntityDBID:
            case MemoryFieldType::ParticleDBID:
            case MemoryFieldType::StringsTableID:
            {
                uint32_t value = Read<uint32_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::Float:
            {
                uint32_t tmp = Script::Memory::ReadDword(field.memoryAddr);
                float value = reinterpret_cast<float&>(tmp);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::Double:
            {
                uint64_t tmp = Script::Memory::ReadQword(field.memoryAddr);
                double value = reinterpret_cast<double&>(tmp);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::Qword:
            {
                int64_t value = Read<int64_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
            case MemoryFieldType::UnsignedQword:
            {
                uint64_t value = Read<uint64_t>(field.memoryAddr);
                mSamples[field.uuid].emplace_back(value);
                break;
            }
        }
    }
}

void S2Plugin::Logger::durationEnded()
{
    mSampleTimer->stop();
    emit samplingEnded();
}

std::pair<int64_t, int64_t> S2Plugin::Logger::sampleBounds(const LoggerField& field) const
{
    int64_t highest = std::numeric_limits<int64_t>::min(); // for all intents and purposes, the values should reasonably all fit in 2^64 / 2
    int64_t lowest = std::numeric_limits<int64_t>::max();
    if (mSamples.count(field.uuid) > 0)
    {
        const auto& samples = mSamples.at(field.uuid);
        for (const auto& value : samples)
        {
            if (field.type == MemoryFieldType::Float)
            {
                int64_t v_ceil = std::ceil(std::any_cast<float>(value));
                int64_t v_floor = std::floor(std::any_cast<float>(value));
                if (v_ceil > highest)
                {
                    highest = v_ceil;
                }
                if (v_floor < lowest)
                {
                    lowest = v_floor;
                }
            }
            else if (field.type == MemoryFieldType::Double)
            {
                int64_t v_ceil = std::ceil(std::any_cast<double>(value));
                int64_t v_floor = std::floor(std::any_cast<double>(value));
                if (v_ceil > highest)
                {
                    highest = v_ceil;
                }
                if (v_floor < lowest)
                {
                    lowest = v_floor;
                }
            }
            else
            {
                int64_t v = 0;
                switch (field.type)
                {
                    case MemoryFieldType::Byte:
                    case MemoryFieldType::State8:
                    {
                        v = std::any_cast<int8_t>(value);
                        break;
                    }
                    case MemoryFieldType::UnsignedByte:
                    case MemoryFieldType::Bool:
                    case MemoryFieldType::Flags8:
                    case MemoryFieldType::CharacterDBID:
                    {
                        v = std::any_cast<uint8_t>(value);
                        break;
                    }
                    case MemoryFieldType::Word:
                    case MemoryFieldType::State16:
                    {
                        v = std::any_cast<int16_t>(value);
                        break;
                    }
                    case MemoryFieldType::UnsignedWord:
                    case MemoryFieldType::Flags16:
                    {
                        v = std::any_cast<uint16_t>(value);
                        break;
                    }
                    case MemoryFieldType::Dword:
                    case MemoryFieldType::State32:
                    case MemoryFieldType::EntityUID:
                    case MemoryFieldType::TextureDBID:
                    {
                        v = std::any_cast<int32_t>(value);
                        break;
                    }
                    case MemoryFieldType::UnsignedDword:
                    case MemoryFieldType::Flags32:
                    case MemoryFieldType::EntityDBID:
                    case MemoryFieldType::ParticleDBID:
                    case MemoryFieldType::StringsTableID:
                    {
                        v = std::any_cast<uint32_t>(value);
                        break;
                    }
                    case MemoryFieldType::Qword:
                    {
                        v = std::any_cast<int64_t>(value);
                        break;
                    }
                    case MemoryFieldType::UnsignedQword:
                    {
                        v = static_cast<int64_t>(std::any_cast<uint64_t>(value));
                        // [Known Issue]: there is no way to represent all unsigned 64bit values in signed 64bit variable
                        // and standard does not provide bigger type to properly compare and determinate biggest and lowest values from the two types
                        if (v < 0)
                            v = std::numeric_limits<int64_t>::max();
                        break;
                    }
                }
                if (v > highest)
                    highest = v;

                if (v < lowest)
                    lowest = v;
            }
        }
    }
    return std::make_pair(lowest, highest);
}
