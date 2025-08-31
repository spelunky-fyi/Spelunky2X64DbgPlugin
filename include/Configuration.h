#pragma once

#include "data/IDNameList.h"
#include <QColor>
#include <QMetaEnum>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    constexpr uint8_t gsColField = 0;
    constexpr uint8_t gsColValue = 1;
    constexpr uint8_t gsColValueHex = 2;
    constexpr uint8_t gsColComparisonValue = 3;
    constexpr uint8_t gsColComparisonValueHex = 4;
    constexpr uint8_t gsColMemoryAddress = 5;
    constexpr uint8_t gsColMemoryAddressDelta = 6;
    constexpr uint8_t gsColType = 7;
    constexpr uint8_t gsColComment = 8;

    /*
     * [[ Roles explanation: ]]
     * The first 5 roles are all saved to the name field
     * those are used as information about the row
     * memory address in the name field are used just for row update and shouldn't really be used for anything else
     *
     * value, comparison value, memoryAddress and delta fields all should contain the `gsRoleRawValue` data
     * (may differ with some special types)
     *
     * valueHex and comparison valueHex contain `gsRoleRawValue` only when it's a pointer (used for update check and click event)
     * value and comparison value also contain `gsRoleMemoryAddress` for field editing purposes
     * for pointers, that will be the pointer value, not memory address of the pointer
     *
     * The rest of the roles are type specific
     */

    constexpr uint16_t gsRoleType = Qt::UserRole + 0;
    constexpr uint16_t gsRoleMemoryAddress = Qt::UserRole + 1;
    constexpr uint16_t gsRoleComparisonMemoryAddress = Qt::UserRole + 2;
    constexpr uint16_t gsRoleIsPointer = Qt::UserRole + 3;
    constexpr uint16_t gsRoleUID = Qt::UserRole + 4;
    constexpr uint16_t gsRoleRawValue = Qt::UserRole + 5;

    constexpr uint16_t gsRoleFlagIndex = Qt::UserRole + 6;
    constexpr uint16_t gsRoleRefName = Qt::UserRole + 7; // ref name for flags, states and vtable
    constexpr uint16_t gsRoleStdContainerFirstParameterType = Qt::UserRole + 8;
    constexpr uint16_t gsRoleStdContainerSecondParameterType = Qt::UserRole + 9;
    constexpr uint16_t gsRoleSize = Qt::UserRole + 10;
    constexpr uint16_t gsRoleColumns = Qt::UserRole + 11;       // for Matrix
    constexpr uint16_t gsRoleEntityAddress = Qt::UserRole + 12; // for entity uid to not look for the uid twice and to open the right entity when clicked on uid

    // new types need to be added to
    // - the MemoryFieldType enum
    // - gsMemoryFieldTypeData in Configuration.cpp
    // - optionally in Structs.json if they have static structure
    // - handling of the json is done in populateMemoryField in Configuration.cpp
    // - displaying the data and handling the click event is done in TreeViewMemoryFields.cpp
    // - if it's common use/basic type, you may also want to add it in getAlignment function
    // - there are some specific conditions for comparison in database handled in DatabaseHelper.cpp
    // new subclasses of Entity can just be added to the class hierarchy in EntitySubclasses.json
    // and have its fields defined there

    enum class MemoryFieldType
    {
        None = 0, // special type just for error handling
        Dummy,    // dummy type for uses like fake parent type in StdMap
        CodePointer,
        DataPointer,
        OnHeapPointer,
        Byte,
        UnsignedByte,
        Word,
        UnsignedWord,
        Dword,
        UnsignedDword,
        Qword,
        UnsignedQword,
        Float,
        Bool,
        Flag,
        Flags32,
        Flags16,
        Flags8,
        State8,  // this is signed, can be negative!
        State16, // this is signed, can be negative!
        State32, // this is signed, can be negative!
        Skip,
        GameManager,
        GameAPI,
        Hud,
        EntityFactory,
        State,
        SaveGame,
        LevelGen,
        LiquidPhysics,
        LiquidPhysicsPointer,
        DebugSettings,
        EntityDB,
        EntityPointer,
        EntityDBPointer,
        EntityDBID,
        EntityUID,
        ParticleDB,
        ParticleDBID,
        ParticleDBPointer,
        TextureDB,
        TextureDBID,
        TextureDBPointer,
        StdVector,
        StdMap,
        ConstCharPointer,
        StdString,
        StdWstring,
        EntitySubclass,               // a subclass of an entity defined in json
        DefaultStructType,            // a struct defined in json
        UndeterminedThemeInfoPointer, // used to look up the theme pointer in the levelGen and show the correct theme name
        COThemeInfoPointer,           // same as above, but does not add struct tree
        LevelGenRoomsPointer,         // used to make the level gen rooms title clickable
        LevelGenRoomsMetaPointer,     // used to make the level gen rooms title clickable
        JournalPagePointer,           // used to make journal page in vector clickable
        LevelGenPointer,
        UTF8Char,
        UTF16Char,
        UTF16StringFixedSize,
        UTF8StringFixedSize,
        StringsTableID,
        CharacterDB,
        CharacterDBID,
        VirtualFunctionTable,
        Online,
        IPv4Address,
        Double,
        Array,
        Matrix,
        EntityList,
        OldStdList,
        StdList,
        StdUnorderedMap,
    };

    struct VirtualFunction
    {
        size_t index;
        std::string name;
        std::string params;
        std::string returnValue;
        std::string type;
        std::string comment;

        VirtualFunction(size_t i, std::string n, std::string p, std::string r, std::string t, std::string c) : index(i), name(n), params(p), returnValue(r), type(t), comment(c){};
        bool operator<(const VirtualFunction& other) const
        {
            return index < other.index;
        }
    };

    struct MemoryField // TODO this got big over time, consider size optimizations
    {
        std::string name;
        MemoryFieldType type{MemoryFieldType::None};
        bool isPointer{false};

        // jsonName only if applicable: if a type is not a MemoryFieldType, but fully defined in the json file
        // then save its name so we can compare later
        std::string jsonName;
        // parameter types for stuff like vectors, maps etc.
        std::string firstParameterType;
        std::string secondParameterType;
        std::string comment;
        // size in bytes
        size_t get_size() const;
        union
        {
            // length, size of array etc.
            size_t numberOfElements{0};
            // row count for matrix
            size_t rows;
        };
        // For checking duplicate names
        bool operator==(const MemoryField& other) const
        {
            return name == other.name;
        };
        void setNumColumns(size_t num)
        {
            columns = num;
        };
        size_t getNumColumns() const
        {
            return columns;
        };

      private:
        // column count for matrix
        size_t columns{0};
        size_t size{0};
        friend class Configuration;
    };

    struct RoomCode
    {
        uint16_t id;
        std::string name;
        std::string enumName;
        QColor color;
        RoomCode(uint16_t _id, std::string _name, std::string _enum, QColor _color) : id(_id), name(_name), enumName(_enum), color(_color){};
    };

    Q_DECLARE_METATYPE(S2Plugin::MemoryFieldType);
    Q_DECLARE_METATYPE(std::string);

    class Configuration
    {
      public:
        static Configuration* get();
        static bool reload();
        // tries to load Configuration if not loaded already
        static bool is_loaded()
        {
            return get() != nullptr;
        }
        // Accessors
        const std::unordered_map<std::string, std::string>& entityClassHierarchy() const noexcept
        {
            return mEntityClassHierarchy;
        }
        const std::vector<std::pair<std::string, std::string>>& defaultEntityClassTypes() const noexcept
        {
            return mDefaultEntityClassTypes;
        }
        const EntityNamesList& entityList() const noexcept
        {
            return entityNames;
        }
        const ParticleEmittersList& particleEmittersList() const noexcept
        {
            return particleEmitters;
        }
        const std::vector<std::string>& getJournalPageNames() const noexcept
        {
            return mJournalPages;
        }
        bool isJournalPage(const std::string& typeName) const
        {
            return std::find(mJournalPages.begin(), mJournalPages.end(), typeName) != mJournalPages.end();
        }
        //
        std::vector<std::string> classHierarchyOfEntity(const std::string& entityName) const;

        const std::vector<MemoryField>& typeFields(const MemoryFieldType type) const;
        const std::vector<MemoryField>& typeFieldsOfEntitySubclass(const std::string& type) const;
        const std::vector<MemoryField>& typeFieldsOfDefaultStruct(const std::string& type) const;
        const std::vector<VirtualFunction>& virtualFunctionsOfType(const std::string& field, bool quiet = false) const;

        // "Entity" returns true, even though it's base class
        bool isEntitySubclass(const std::string& type) const;

        static MemoryFieldType getBuiltInType(const std::string& type);
        static std::string_view getCPPTypeName(MemoryFieldType type);
        static std::string_view getTypeDisplayName(MemoryFieldType type);
        static size_t getBuiltInTypeSize(MemoryFieldType type);
        static bool isPointerType(MemoryFieldType type);
        MemoryField nameToMemoryField(const std::string& typeName) const;

        uintptr_t offsetForField(const std::vector<MemoryField>& fields, std::string_view fieldUID, uintptr_t base_addr = 0) const;
        uintptr_t offsetForField(MemoryFieldType type, std::string_view fieldUID, uintptr_t base_addr = 0) const;

        // equivalent to alignof operator
        uint8_t getAlignment(const std::string& type) const;
        uint8_t getAlignment(const MemoryField& type) const;

      private:
        uint8_t getAlignment(MemoryFieldType type) const;

      public:
        std::string flagTitle(const std::string& fieldName, uint8_t flagNumber) const;
        std::string stateTitle(const std::string& fieldName, int64_t state) const;
        const std::vector<std::pair<int64_t, std::string>>& refTitlesOfField(const std::string& fieldName) const;

        size_t getTypeSize(const std::string& typeName, bool entitySubclass = false);

        const RoomCode& roomCodeForID(uint16_t code) const;
        std::string getEntityName(uint32_t type) const;

        bool isPermanentPointer(const std::string& type) const
        {
            return std::find(mPointerTypes.begin(), mPointerTypes.end(), type) != mPointerTypes.end();
        }
        bool isPermanentPointer(const std::string_view type) const
        {
            return std::find(mPointerTypes.begin(), mPointerTypes.end(), type) != mPointerTypes.end();
        }
        bool isJsonStruct(const std::string& type) const
        {
            return mTypeFieldsStructs.find(type) != mTypeFieldsStructs.end();
        }

      private:
        static Configuration* ptr;
        bool initializedCorrectly = false;

        std::unordered_map<std::string, std::string> mEntityClassHierarchy;
        std::vector<std::pair<std::string, std::string>> mDefaultEntityClassTypes;

        // for build in types defined in json
        std::unordered_map<MemoryFieldType, std::vector<MemoryField>> mTypeFieldsMain;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsEntitySubclasses;
        std::unordered_map<std::string, std::vector<MemoryField>> mTypeFieldsStructs;
        std::vector<std::string> mPointerTypes; // pointers defined in pointer_types in json

        std::vector<std::string> mJournalPages;

        std::unordered_map<std::string, size_t> mTypeFieldsStructsSizes;

        std::unordered_map<std::string, std::vector<VirtualFunction>> mVirtualFunctions;
        std::unordered_map<std::string, uint8_t> mAlignments;
        std::unordered_map<std::string, std::vector<std::pair<int64_t, std::string>>> mRefs; // for flags and states

        std::unordered_map<uint16_t, RoomCode> mRoomCodes;

        void processEntitiesJSON(nlohmann::ordered_json& json);
        void processJSON(nlohmann::ordered_json& json);
        void processRoomCodesJSON(nlohmann::ordered_json& json);
        MemoryField populateMemoryField(const nlohmann::ordered_json& field, const std::string& struct_name);

        EntityNamesList entityNames;
        ParticleEmittersList particleEmitters;

        Configuration();
        ~Configuration(){};
        Configuration(const Configuration&) = delete;
        Configuration& operator=(const Configuration&) = delete;

        friend class ItemModelVirtualFunctions;
    };
} // namespace S2Plugin
