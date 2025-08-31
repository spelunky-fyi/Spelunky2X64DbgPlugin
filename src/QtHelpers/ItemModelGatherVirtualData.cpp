#include "QtHelpers/ItemModelGatherVirtualData.h"

#include "Configuration.h"
#include "JsonNameDefinitions.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

enum VIRT_FUNC : uint32_t
{
    ENTITY_STATEMACHINE = 2,
    ENTITY_KILL = 3,
    ENTITY_COLLISION1 = 4,
    ENTITY_DESTROY = 5,
    ENTITY_OPEN = 24,
    ENTITY_COLLISION2 = 26,
    MOVABLE_DAMAGE = 48,
};

QVariant S2Plugin::ItemModelGatherVirtualData::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        const auto& entry = mEntries.at(static_cast<size_t>(index.row()));
        switch (index.column())
        {
            case gsColGatherID:
                return entry.id;
            case gsColGatherName:
                return entry.name;
            case gsColGatherVirtualTableOffset:
                return entry.virtualTableOffset;
            case gsColGatherCollision1Present:
                return entry.collision1Present ? "Yes" : "";
            case gsColGatherCollision2Present:
                return entry.collision2Present ? "Yes" : "";
            case gsColGatherOpenPresent:
                return entry.openPresent ? "Yes" : "";
            case gsColGatherDamagePresent:
                return entry.damagePresent ? "Yes" : "";
            case gsColGatherKillPresent:
                return entry.killPresent ? "Yes" : "";
            case gsColGatherDestroyPresent:
                return entry.destroyPresent ? "Yes" : "";
            case gsColGatherStatemachinePresent:
                return entry.statemachinePresent ? "Yes" : "";
        }
    }
    return QVariant();
}

QVariant S2Plugin::ItemModelGatherVirtualData::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
            case gsColGatherID:
                return "ID";
            case gsColGatherName:
                return "Name";
            case gsColGatherVirtualTableOffset:
                return "Vftable start";
            case gsColGatherCollision1Present:
                return "Collision 1";
            case gsColGatherCollision2Present:
                return "Collision 2";
            case gsColGatherOpenPresent:
                return "Open";
            case gsColGatherDamagePresent:
                return "Damage";
            case gsColGatherKillPresent:
                return "Kill";
            case gsColGatherDestroyPresent:
                return "Destroy";
            case gsColGatherStatemachinePresent:
                return "State Machine";
        }
    }
    return QVariant();
}

void S2Plugin::ItemModelGatherVirtualData::gatherEntities()
{
    auto spel2 = Spelunky2::get();
    auto config = Configuration::get();
    auto& vtl = spel2->get_VirtualTableLookup();
    auto statePtr = spel2->get_StatePtr(false);
    if (statePtr == 0)
        return;

    auto processEntities = [&](size_t layerEntities, uint32_t count)
    {
        uint32_t maximum = (std::min)(count, 10000u);
        for (uint32_t x = 0; x < maximum; ++x)
        {
            auto entityPtr = layerEntities + (x * sizeof(size_t));
            auto entity = Script::Memory::ReadQword(entityPtr);
            auto entityVTableOffset = Script::Memory::ReadQword(entity);

            auto entityType = Script::Memory::ReadDword(Script::Memory::ReadQword(entity + 0x8) + 0x14);

            for (auto& entry : mEntries)
            {
                if (entry.virtualTableOffset == 0 && entry.id == entityType)
                {
                    auto tableOffset = (entityVTableOffset - vtl.tableStartAddress()) / sizeof(size_t);
                    entry.virtualTableOffset = tableOffset;
                }
            }
        }
    };

    beginResetModel();
    auto layer0Offset = config->offsetForField(MemoryFieldType::State, JsonName::Layer0, statePtr);
    auto layer0 = Script::Memory::ReadQword(layer0Offset);
    auto layer0Count = Script::Memory::ReadDword(layer0 + 28);
    auto layer0Entities = Script::Memory::ReadQword(layer0 + 8);
    processEntities(layer0Entities, layer0Count);

    auto layer1Offset = config->offsetForField(MemoryFieldType::State, JsonName::Layer1, statePtr);
    auto layer1 = Script::Memory::ReadQword(layer1Offset);
    auto layer1Count = Script::Memory::ReadDword(layer1 + 28);
    auto layer1Entities = Script::Memory::ReadQword(layer1 + 8);
    processEntities(layer1Entities, layer1Count);
    endResetModel();
}

void S2Plugin::ItemModelGatherVirtualData::gatherExtraObjects()
{
    beginResetModel();
    auto spel2 = Spelunky2::get();
    auto config = Configuration::get();
    auto& vtl = spel2->get_VirtualTableLookup();
    size_t index;
    uint8_t counter = 0;

    if (auto levelGenPtr = spel2->get_LevelGenPtr(false); levelGenPtr != 0)
    {
        // names only used for GatheredDataEntry
        static const auto themes = {"THEME_DWELLING", "THEME_JUNGLE",     "THEME_VOLCANA",       "THEME_OLMEC",       "THEME_TIDEPOOL",     "THEME_TEMPLE",
                                    "THEME_ICECAVES", "THEME_NEOBABYLON", "THEME_SUNKENCITY",    "THEME_COSMICOCEAN", "THEME_CITY_OF_GOLD", "THEME_DUAT",
                                    "THEME_ABZU",     "THEME_TIAMAT",     "THEME_EGGPLANTWORLD", "THEME_HUNDUN",      "THEME_BASECAMP",     "THEME_ARENA"};
        index = 1000;
        auto firstThemeOffset = config->offsetForField(config->typeFields(MemoryFieldType::LevelGen), JsonName::ThemeDwelling, levelGenPtr);
        for (const auto themeName : themes)
        {
            auto themeAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(firstThemeOffset + counter * sizeof(uintptr_t)));
            auto tableOffset = (themeAddress - vtl.tableStartAddress()) / sizeof(size_t);
            bool foundInEntries = false;
            for (auto& entry : mEntries)
            {
                if (entry.id == index)
                {
                    entry.virtualTableOffset = tableOffset;
                    foundInEntries = true;
                }
            }
            if (!foundInEntries)
            {
                auto g = GatheredDataEntry();
                g.id = index;
                g.name = themeName;
                g.virtualTableOffset = tableOffset;
                g.collision1Present = false;
                g.collision2Present = false;
                mEntries.emplace_back(std::move(g));
            }
            index++;
            counter++;
        }
    }

    uintptr_t statePtr = spel2->get_StatePtr(false);
    if (statePtr != 0)
    {
        std::vector<std::string> logics;
        uintptr_t firstLogicPtr;
        { // get the names and address of the logics
            auto& stateFields = config->typeFields(MemoryFieldType::State);
            size_t delta = 0;
            std::string logicTypeName;
            for (auto& field : stateFields)
            {
                if (field.name == JsonName::StateLogicFieldName)
                {
                    logicTypeName = field.jsonName;
                    break;
                }
                delta += field.get_size();
            }
            auto& logicListFields = config->typeFieldsOfDefaultStruct(logicTypeName);
            logics.reserve(logicListFields.size());
            for (auto& field : logicListFields)
                logics.push_back(field.name);

            firstLogicPtr = Script::Memory::ReadQword(statePtr + delta);
        }

        index = 2000;
        counter = 0;
        for (const auto& logicName : logics)
        {
            auto logicAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(firstLogicPtr + counter * sizeof(uintptr_t)));
            size_t tableOffset = 0;
            if (logicAddress != 0)
            {
                tableOffset = (logicAddress - vtl.tableStartAddress()) / sizeof(size_t);
            }
            bool foundInEntries = false;
            for (auto& entry : mEntries)
            {
                if (entry.id == index)
                {
                    if (tableOffset != 0)
                    {
                        entry.virtualTableOffset = tableOffset;
                    }
                    foundInEntries = true;
                }
            }
            if (!foundInEntries)
            {
                auto g = GatheredDataEntry();
                g.id = index;
                std::string logicNameUpper = "LOGIC_";
                logicNameUpper.resize(logicName.size() + 6);
                std::transform(logicName.begin(), logicName.end(), logicNameUpper.begin() + 6, ::toupper);
                g.name = QString::fromStdString(logicNameUpper);
                g.virtualTableOffset = tableOffset;
                g.collision1Present = false;
                g.collision2Present = false;
                mEntries.emplace_back(std::move(g));
            }
            index++;
            counter++;
        }
    }
    if (auto gameManagerPtr = spel2->get_GameManagerPtr(); gameManagerPtr != 0)
    {
        index = 3000;
        for (const auto& screenName : JsonName::ScreensGamemanager)
        {
            auto offset = config->offsetForField(config->typeFields(MemoryFieldType::GameManager), screenName, gameManagerPtr);
            auto screenAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(offset));
            size_t tableOffset = 0;
            if (screenAddress != 0)
            {
                tableOffset = (screenAddress - vtl.tableStartAddress()) / sizeof(size_t);
            }
            bool foundInEntries = false;
            for (auto& entry : mEntries)
            {
                if (entry.id == index)
                {
                    if (tableOffset != 0)
                    {
                        entry.virtualTableOffset = tableOffset;
                    }
                    foundInEntries = true;
                }
            }
            if (!foundInEntries)
            {
                auto g = GatheredDataEntry();
                g.id = index;
                std::string screenNameUpper;
                screenNameUpper.resize(screenName.length());
                std::transform(screenName.begin(), screenName.end(), screenNameUpper.begin(), ::toupper);
                g.name = QString::fromStdString(screenNameUpper);
                g.virtualTableOffset = tableOffset;
                g.collision1Present = false;
                g.collision2Present = false;
                mEntries.emplace_back(std::move(g));
            }
            index++;
        }
    }
    if (statePtr != 0)
    {
        index = 3500;
        for (const auto& screenName : JsonName::ScreensState)
        {
            auto screenAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(config->offsetForField(config->typeFields(MemoryFieldType::State), screenName, statePtr)));
            size_t tableOffset = 0;
            if (screenAddress != 0)
            {
                tableOffset = (screenAddress - vtl.tableStartAddress()) / sizeof(size_t);
            }
            bool foundInEntries = false;
            for (auto& entry : mEntries)
            {
                if (entry.id == index)
                {
                    if (tableOffset != 0)
                    {
                        entry.virtualTableOffset = tableOffset;
                    }
                    foundInEntries = true;
                }
            }
            if (!foundInEntries)
            {
                auto g = GatheredDataEntry();
                g.id = index;
                std::string screenNameUpper;
                screenNameUpper.resize(screenName.length());
                std::transform(screenName.begin(), screenName.end(), screenNameUpper.begin(), ::toupper);
                g.name = QString::fromStdString(screenNameUpper);
                g.virtualTableOffset = tableOffset;
                g.collision1Present = false;
                g.collision2Present = false;
                mEntries.emplace_back(std::move(g));
            }
            index++;
        }

        index = 4000;
        for (const auto& questName : JsonName::QuestNames)
        {
            auto questAddress = Script::Memory::ReadQword(Script::Memory::ReadQword(config->offsetForField(config->typeFields(MemoryFieldType::State), questName, statePtr)));
            size_t tableOffset = 0;
            if (questAddress != 0)
            {
                tableOffset = (questAddress - vtl.tableStartAddress()) / sizeof(size_t);
            }
            bool foundInEntries = false;
            for (auto& entry : mEntries)
            {
                if (entry.id == index)
                {
                    if (tableOffset != 0)
                    {
                        entry.virtualTableOffset = tableOffset;
                    }
                    foundInEntries = true;
                }
            }
            if (!foundInEntries)
            {
                auto g = GatheredDataEntry();
                g.id = index;
                auto questNameSV = questName.substr(JsonName::Quests.size() + 1); // omit the "quests."
                std::string questNameUpper;
                questNameUpper.resize(questNameSV.length());
                std::transform(questNameSV.begin(), questNameSV.end(), questNameUpper.begin(), ::toupper);
                g.name = QString("QUEST_") + QString::fromStdString(questNameUpper);
                g.virtualTableOffset = tableOffset;
                g.collision1Present = false;
                g.collision2Present = false;
                mEntries.emplace_back(std::move(g));
            }
            index++;
        }
    }
    endResetModel();
}

float S2Plugin::ItemModelGatherVirtualData::completionPercentage() const
{
    float completed = 0.0;
    for (const auto& e : mEntries)
    {
        if (e.virtualTableOffset != 0)
        {
            completed += 1.0;
        }
    }
    return (completed / mEntries.size()) * 100;
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpJSON() const
{
    nlohmann::json root;
    for (const auto& e : mEntries)
    {
        nlohmann::json j;
        j["id"] = e.id;
        j["name"] = e.name.toStdString();
        j["offset"] = e.virtualTableOffset;
        j["collision1"] = e.collision1Present;
        j["collision2"] = e.collision2Present;
        j["open"] = e.openPresent;
        j["damage"] = e.damagePresent;
        j["kill"] = e.killPresent;
        j["destroy"] = e.destroyPresent;
        j["statemachine"] = e.statemachinePresent;

        root.push_back(j);
    }
    return root.dump(4);
}

void S2Plugin::ItemModelGatherVirtualData::parseJSON()
{
    using nlohmann::ordered_json;

    char buffer[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    auto pathQStr = QFileInfo(QString(buffer)).dir().filePath(QString::fromStdString("plugins/spel2/VirtualTableData.json"));
    if (QFile(pathQStr).exists())
    {
        try
        {
            std::ifstream fp(pathQStr.toStdString());
            std::string jsonString((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
            auto j = ordered_json::parse(jsonString, nullptr, true, true);

            for (const auto& jsonEntry : j)
            {
                auto g = GatheredDataEntry();
                g.id = jsonEntry["id"].get<uint32_t>();
                g.name = QString::fromStdString(jsonEntry["name"].get<std::string>());
                g.virtualTableOffset = jsonEntry["offset"].get<size_t>();
                g.collision1Present = jsonEntry["collision1"].get<bool>();
                g.collision2Present = jsonEntry["collision2"].get<bool>();
                g.openPresent = jsonEntry["open"].get<bool>();
                g.damagePresent = jsonEntry["damage"].get<bool>();
                g.killPresent = jsonEntry["kill"].get<bool>();
                g.destroyPresent = jsonEntry["destroy"].get<bool>();
                g.statemachinePresent = jsonEntry["statemachine"].get<bool>();
                mEntries.emplace_back(std::move(g));
            }
        }
        catch (const ordered_json::exception& e)
        {
            displayError(("Exception while parsing VirtualTableData.json: " + std::string(e.what())).c_str());
        }
        catch (const std::exception& e)
        {
            displayError(("Exception while parsing VirtualTableData.json: " + std::string(e.what())).c_str());
        }
        catch (...)
        {
            displayError("Unknown exception while parsing VirtualTableData.json");
        }

        std::sort(mEntries.begin(), mEntries.end(), [](const GatheredDataEntry& a, const GatheredDataEntry& b) { return a.id < b.id; });
    }
    else
    {
        auto& entitiesList = Configuration::get()->entityList();
        for (const auto& [entityID, entityName] : entitiesList.entries())
        {
            auto g = GatheredDataEntry();
            g.id = entityID;
            g.name = QString::fromStdString(entityName);
            g.virtualTableOffset = 0;
            g.collision1Present = false;
            g.collision2Present = false;
            mEntries.emplace_back(std::move(g));
        }
    }
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpVirtTable() const
{
    std::stringstream ss;
    ss << "Entity|on_collision1|on_collision2|on_open|on_damage|on_kill|on_destroy|statemachine\n";
    ss << "---|---|---|---|---|---|---|---\n";
    for (const auto& e : mEntries)
    {
        if (e.id < 1000)
        {
            if (e.virtualTableOffset == 0)
            {
                ss << e.name.toStdString() << "|?|?|?|?|?|?|?\n";
            }
            else
            {
                ss << e.name.toStdString() << "|";
                ss << (e.collision1Present ? "col1" : "") << "|";
                ss << (e.collision2Present ? "col2" : "") << "|";
                ss << (e.openPresent ? "open" : "") << "|";
                ss << (e.damagePresent ? "dmg" : "") << "|";
                ss << (e.killPresent ? "kill" : "") << "|";
                ss << (e.destroyPresent ? "dstry" : "") << "|";
                ss << (e.statemachinePresent ? "stmchn" : "");
                ss << "\n";
            }
        }
    }
    return ss.str();
}

std::string S2Plugin::ItemModelGatherVirtualData::dumpCppEnum() const
{
    std::stringstream ss;
    ss << "enum class VTABLE_OFFSET\n";
    ss << "{\n";
    ss << "    NONE = 0,\n";
    for (const auto& e : mEntries)
    {
        if (e.virtualTableOffset == 0)
        {
            ss << "    // " << e.name.toStdString() << " = UNKNOWN\n";
        }
        else
        {
            ss << "    " + e.name.toStdString() << " = " << e.virtualTableOffset << ",\n";
        }
    }
    ss << "};\n";
    return ss.str();
}

void S2Plugin::ItemModelGatherVirtualData::gatherAvailableVirtuals()
{
    auto& vtl = Spelunky2::get()->get_VirtualTableLookup();

    auto isVirtImplemented = [&vtl](size_t functionIndex)
    {
        auto tableAddress = vtl.tableAddressForEntry(vtl.entryForOffset(functionIndex));
        auto functionStart = Script::Memory::ReadQword(tableAddress);
        auto firstByte = Script::Memory::ReadByte(functionStart);

        // Check a simple `ret` opcode
        if (firstByte == 0xC3)
        {
            return false;
        }

        // Check for `xor eax, eax ; ret`
        auto secondByte = Script::Memory::ReadByte(functionStart + 1);
        auto thirdByte = Script::Memory::ReadByte(functionStart + 2);
        if (firstByte == 0x31 && secondByte == 0xC0 && thirdByte == 0xC3)
        {
            return false;
        }

        // Check for `mov al, 1 ; ret`
        if (firstByte == 0xB0 && secondByte == 0x01 && thirdByte == 0xC3)
        {
            return false;
        }

        return true;
    };

    beginResetModel();
    auto config = Configuration::get();
    for (auto& entry : mEntries)
    {
        if (entry.id < 1000 && entry.virtualTableOffset != 0)
        {
            auto entityClassHierarchy = config->classHierarchyOfEntity(entry.name.toStdString());
            bool isMovable = false;
            for (const auto& c : entityClassHierarchy)
            {
                if (c == JsonName::Movable)
                {
                    isMovable = true;
                    break;
                }
            }

            entry.collision1Present = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_COLLISION1);
            entry.collision2Present = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_COLLISION2);
            entry.openPresent = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_OPEN);
            entry.killPresent = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_KILL);
            entry.damagePresent = isMovable ? isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::MOVABLE_DAMAGE) : false;
            entry.destroyPresent = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_DESTROY);
            entry.statemachinePresent = isVirtImplemented(entry.virtualTableOffset + VIRT_FUNC::ENTITY_STATEMACHINE);
        }
    }
    endResetModel();
}
