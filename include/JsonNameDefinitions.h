#pragma once

#include <array>
#include <string_view>

/*
 *  This file contains all the raw strings used in the codebase (except for Configuration.cpp)
 *  that reference field/struct defined in json.
 *  Made for easy editing
 */

namespace S2Plugin
{
    namespace JsonName
    {
        // field names in state:
        constexpr static const std::string_view Layer0EntitiesByMask = "layer0.entities_by_mask";
        constexpr static const std::string_view Layer1EntitiesByMask = "layer1.entities_by_mask";
        constexpr static const std::string_view Layer0GridEntities = "layer0.grid_entities";
        constexpr static const std::string_view Layer1GridEntities = "layer1.grid_entities";
        constexpr static const std::string_view LevelWidthRooms = "level_width_rooms";
        constexpr static const std::string_view LevelHeightRooms = "level_height_rooms";
        constexpr static const std::string_view UidToEntityMask = "uid_to_entity_mask";
        constexpr static const std::string_view Layer0 = "layer0";
        constexpr static const std::string_view Layer1 = "layer1";
        constexpr static const std::string_view StateLogicFieldName = "logic";
        constexpr static const std::string_view StateThemeField = "theme";

        constexpr static const std::string_view ThemeDwelling = "theme_dwelling";    // field name in LevelGen
        constexpr static const std::string_view EntitiesByMask = "entities_by_mask"; // field name in Layer
        constexpr static const std::string_view EntityDBID = "id";                   // field in EntityDB
        // struct names
        constexpr static const std::string_view LayerStructName = "LayerPointer";
        constexpr static const std::string_view Movable = "Movable";
        constexpr static const std::string_view JournalPage = "JournalPage";
        constexpr static const std::string_view Entity = "Entity";
        constexpr static const std::string_view EntityDB = "EntityDB";
        constexpr static const std::string_view ParticleDB = "ParticleDB";
        constexpr static const std::string_view TextureDB = "TextureDB";
        constexpr static const std::string_view CharacterDB = "CharacterDB";
        constexpr static const std::string_view LevelGen = "LevelGen";
        constexpr static const std::string_view ThemeInfo = "ThemeInfo";

        constexpr static auto ScreensGamemanager = std::array<std::string_view, 13>{
            "screen_logo",         "screen_intro",      "screen_prologue", "screen_title", "screen_menu",           "screen_options",      "screen_player_profile",
            "screen_leaderboards", "screen_seed_input", "screen_camp",     "screen_level", "screen_online_loading", "screen_online_lobby",
        };
        constexpr static auto ScreensState = std::array<std::string_view, 15>{
            "screen_character_select",
            "screen_team_select",
            "screen_transition",
            "screen_death",
            "screen_win",
            "screen_credits",
            "screen_scores",
            "screen_constellation",
            "screen_recap",
            "screen_arena_menu",
            "screen_arena_stages_select1",
            "screen_arena_items",
            "screen_arena_intro",
            "screen_arena_level",
            "screen_arena_score",
        };
#define QUESTS "quests"
        constexpr static const std::string_view Quests = QUESTS;
        constexpr static auto QuestNames = std::array<std::string_view, 6>{
            QUESTS "."
                   "yang",
            QUESTS "."
                   "jungle_sisters",
            QUESTS "."
                   "van_horsing",
            QUESTS "."
                   "sparrow",
            QUESTS "."
                   "madame_tusk",
            QUESTS "."
                   "beg",
        };
#undef QUESTS
    }; // namespace JsonName
} // namespace S2Plugin
