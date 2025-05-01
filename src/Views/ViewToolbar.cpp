#include "Views/ViewToolbar.h"

#include "Configuration.h"
#include "Spelunky2.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewEntityFactory.h"
#include "Views/ViewEntityList.h"
#include "Views/ViewJournalPage.h"
#include "Views/ViewLevelGen.h"
#include "Views/ViewLogger.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewSaveStates.h"
#include "Views/ViewStdList.h"
#include "Views/ViewStdMap.h"
#include "Views/ViewStdUnorderedMap.h"
#include "Views/ViewStdVector.h"
#include "Views/ViewStringsTable.h"
#include "Views/ViewStruct.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewVirtualFunctions.h"
#include "Views/ViewVirtualTable.h"
#include "pluginmain.h"
#include <QMdiSubWindow>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

S2Plugin::ViewToolbar::ViewToolbar(QMdiArea* mdiArea, QWidget* parent) : QDockWidget(parent, Qt::WindowFlags()), mMDIArea(mdiArea)
{
    setFeatures(QDockWidget::NoDockWidgetFeatures);

    auto mainLayout = new QVBoxLayout();
    auto container = new QWidget(this);
    container->setLayout(mainLayout);
    setWidget(container);

    auto addDivider = [&mainLayout]()
    {
        auto line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line);
    };

    setTitleBarWidget(new QWidget(this));

    mainLayout->addWidget(new QLabel("Databases:"), 0, Qt::AlignHCenter);

    auto btnEntityDB = new QPushButton("Entity DB", this);
    mainLayout->addWidget(btnEntityDB);
    QObject::connect(btnEntityDB, &QPushButton::clicked, this, &ViewToolbar::showEntityDB);
    auto btnTextureDB = new QPushButton("Texture DB", this);
    mainLayout->addWidget(btnTextureDB);
    QObject::connect(btnTextureDB, &QPushButton::clicked, this, &ViewToolbar::showTextureDB);
    auto btnParticleDB = new QPushButton("Particle DB", this);
    mainLayout->addWidget(btnParticleDB);
    QObject::connect(btnParticleDB, &QPushButton::clicked, this, &ViewToolbar::showParticleDB);
    auto btnStringsTable = new QPushButton("Strings DB", this);
    mainLayout->addWidget(btnStringsTable);
    QObject::connect(btnStringsTable, &QPushButton::clicked, this, &ViewToolbar::showStringsTable);
    auto btnCharacterDB = new QPushButton("Character DB", this);
    mainLayout->addWidget(btnCharacterDB);
    QObject::connect(btnCharacterDB, &QPushButton::clicked, this, &ViewToolbar::showCharacterDB);

    addDivider();
    mainLayout->addWidget(new QLabel("Game structs:"), 0, Qt::AlignHCenter);

    auto btnEntityFactory = new QPushButton("Entity Factory", this);
    mainLayout->addWidget(btnEntityFactory);
    QObject::connect(btnEntityFactory, &QPushButton::clicked, this, &ViewToolbar::showEntityFactory);
    auto btnGameManager = new QPushButton("GameManager", this);
    mainLayout->addWidget(btnGameManager);
    QObject::connect(btnGameManager, &QPushButton::clicked, this, &ViewToolbar::showGameManager);
    auto btnOnline = new QPushButton("Online", this);
    mainLayout->addWidget(btnOnline);
    QObject::connect(btnOnline, &QPushButton::clicked, this, &ViewToolbar::showOnline);
    auto btnVirtualTable = new QPushButton("Virtual Table", this);
    mainLayout->addWidget(btnVirtualTable);
    QObject::connect(btnVirtualTable, &QPushButton::clicked, this, &ViewToolbar::showVirtualTableLookup);
    auto btnGameAPI = new QPushButton("Game API", this);
    mainLayout->addWidget(btnGameAPI);
    QObject::connect(btnGameAPI, &QPushButton::clicked, this, &ViewToolbar::showGameAPI);
    auto btnHud = new QPushButton("Hud", this);
    mainLayout->addWidget(btnHud);
    QObject::connect(btnHud, &QPushButton::clicked, this, &ViewToolbar::showHud);
    auto btnDebugSettings = new QPushButton("Debug Settings", this);
    mainLayout->addWidget(btnDebugSettings);
    QObject::connect(btnDebugSettings, &QPushButton::clicked, this, &ViewToolbar::showDebugSettings);
    auto btnSaveStates = new QPushButton("Save States", this);
    btnSaveStates->setToolTip("In online, game saves the game state for potential rollback");
    mainLayout->addWidget(btnSaveStates);
    QObject::connect(btnSaveStates, &QPushButton::clicked, this, &ViewToolbar::showSaveStates);

    addDivider();
    mainLayout->addWidget(new QLabel("Main Thread heap:", this), 0, Qt::AlignHCenter);

    auto btnState = new QPushButton("State", this);
    mainLayout->addWidget(btnState);
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showMainThreadState);
    auto btnEntities = new QPushButton("Entities", this);
    btnEntities->setToolTip("Lookup entities in current level");
    mainLayout->addWidget(btnEntities);
    QObject::connect(btnEntities, &QPushButton::clicked, this, &ViewToolbar::showEntities);
    auto btnLevelGen = new QPushButton("LevelGen", this);
    mainLayout->addWidget(btnLevelGen);
    QObject::connect(btnLevelGen, &QPushButton::clicked, this, &ViewToolbar::showMainThreadLevelGen);
    auto btnLiquid = new QPushButton("Liquid Physics", this);
    mainLayout->addWidget(btnLiquid);
    QObject::connect(btnLiquid, &QPushButton::clicked, this, &ViewToolbar::showMainThreadLiquidPhysics);
    auto btnSaveGame = new QPushButton("SaveGame", this);
    mainLayout->addWidget(btnSaveGame);
    QObject::connect(btnSaveGame, &QPushButton::clicked, this, &ViewToolbar::showMainThreadSaveGame);

    mainLayout->addStretch();
    addDivider();
    mainLayout->addWidget(new QLabel("Tools:"), 0, Qt::AlignHCenter);

    auto btnLogger = new QPushButton("Logger", this);
    btnLogger->setToolTip("Log data over time");
    mainLayout->addWidget(btnLogger);
    QObject::connect(btnLogger, &QPushButton::clicked, this, &ViewToolbar::showLogger);
    auto btnClearLabels = new QPushButton("Clear labels", this);
    btnClearLabels->setToolTip("Clear all labels crated by the plugin (auto labels)");
    mainLayout->addWidget(btnClearLabels);
    QObject::connect(btnClearLabels, &QPushButton::clicked, this, &ViewToolbar::clearLabels);
    auto btnReloadConfig = new QPushButton("Reload JSON", this);
    btnReloadConfig->setToolTip("Reload config from Spelunky2.json and Spelunky2Entities.json");
    mainLayout->addWidget(btnReloadConfig);
    QObject::connect(btnReloadConfig, &QPushButton::clicked, this, &ViewToolbar::reloadConfig);
}

void S2Plugin::ViewToolbar::showVirtualFunctions(uintptr_t address, const std::string& typeName)
{
    auto w = new ViewVirtualFunctions(address, typeName);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdVector(uintptr_t address, const std::string& typeName)
{
    auto w = new ViewStdVector(address, typeName);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdMap(uintptr_t address, const std::string& keyTypeName, const std::string& valueTypeName)
{
    auto w = new ViewStdMap(address, keyTypeName, valueTypeName);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdUnorderedMap(uintptr_t address, const std::string& keyTypeName, const std::string& valueTypeName)
{
    auto w = new ViewStdUnorderedMap(address, keyTypeName, valueTypeName);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showJournalPage(uintptr_t address)
{
    auto w = new ViewJournalPage(address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showState(uintptr_t address)
{
    auto w = new ViewStruct(address, Configuration::get()->typeFields(MemoryFieldType::State), "State");
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showLevelGen(uintptr_t address)
{
    auto w = new ViewLevelGen(address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showLiquidPhysics(uintptr_t address)
{
    auto w = new ViewStruct(address, Configuration::get()->typeFields(MemoryFieldType::LiquidPhysics), "LiquidPhysics");
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showEntity(uintptr_t address)
{
    auto w = new ViewEntity(address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showArray(uintptr_t address, std::string name, std::string arrayTypeName, size_t length)
{
    auto w = new ViewArray(address, std::move(arrayTypeName), length, std::move(name));
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showMatrix(uintptr_t address, std::string name, std::string arrayTypeName, size_t rows, size_t columns)
{
    auto w = new ViewMatrix(address, std::move(arrayTypeName), rows, columns, std::move(name));
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showEntityList(uintptr_t address)
{
    auto w = new ViewEntityList(address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdList(uintptr_t address, std::string valueType, bool isOldType)
{
    auto w = new ViewStdList(address, std::move(valueType), isOldType);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showSaveGame(uintptr_t address)
{
    auto w = new ViewStruct(address, Configuration::get()->typeFields(MemoryFieldType::SaveGame), "SaveGame");
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

//
// slots:
//

S2Plugin::ViewEntityDB* S2Plugin::ViewToolbar::showEntityDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_EntityDB().isValid())
    {
        auto w = new ViewEntityDB();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewParticleDB* S2Plugin::ViewToolbar::showParticleDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_ParticleDB().isValid())
    {
        auto w = new ViewParticleDB();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewTextureDB* S2Plugin::ViewToolbar::showTextureDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_TextureDB().isValid())
    {
        auto w = new ViewTextureDB();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
        return w;
    }
    return nullptr;
}

S2Plugin::ViewCharacterDB* S2Plugin::ViewToolbar::showCharacterDB()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_CharacterDB().isValid())
    {
        auto w = new ViewCharacterDB();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showMainThreadState()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto statePtr = Spelunky2::get()->get_StatePtr(false);
        if (statePtr != 0)
            showState(statePtr);
    }
}

void S2Plugin::ViewToolbar::showMainThreadLevelGen()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto levelGenPtr = Spelunky2::get()->get_LevelGenPtr(false);
        if (levelGenPtr != 0)
            showLevelGen(levelGenPtr);
    }
}

void S2Plugin::ViewToolbar::showMainThreadLiquidPhysics()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_LiquidEnginePtr(false);
        if (ptr != 0)
            showLiquidPhysics(ptr);
    }
}

void S2Plugin::ViewToolbar::showGameManager()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_GameManagerPtr();
        if (ptr != 0)
        {
            auto w = new ViewStruct(ptr, Configuration::get()->typeFields(MemoryFieldType::GameManager), "GameManager");
            auto win = mMDIArea->addSubWindow(w);
            win->setVisible(true);
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
    }
}

S2Plugin::ViewVirtualTable* S2Plugin::ViewToolbar::showVirtualTableLookup()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_VirtualTableLookup().isValid())
    {
        auto w = new ViewVirtualTable();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
        return w;
    }
    return nullptr;
}

void S2Plugin::ViewToolbar::showStringsTable()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_StringsTable(false).isValid())
    {
        auto w = new ViewStringsTable();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showOnline()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_OnlinePtr();
        if (ptr != 0)
        {
            auto w = new ViewStruct(ptr, Configuration::get()->typeFields(MemoryFieldType::Online), "Online");
            auto win = mMDIArea->addSubWindow(w);
            win->setVisible(true);
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
    }
}

void S2Plugin::ViewToolbar::showEntities()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_HeapBase(false) != 0)
    {
        auto w = new ViewEntities();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showMainThreadSaveGame()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_SaveDataPtr(false);
        if (ptr != 0)
            showSaveGame(ptr);
    }
}

void S2Plugin::ViewToolbar::showGameAPI()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_GameAPIPtr();
        if (ptr != 0)
        {
            auto w = new ViewStruct(ptr, Configuration::get()->typeFields(MemoryFieldType::GameAPI), "GameAPI");
            auto win = mMDIArea->addSubWindow(w);
            win->setVisible(true);
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
    }
}

void S2Plugin::ViewToolbar::showHud()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_HudPtr();
        if (ptr != 0)
        {
            auto w = new ViewStruct(ptr, Configuration::get()->typeFields(MemoryFieldType::Hud), "Hud");
            auto win = mMDIArea->addSubWindow(w);
            win->setVisible(true);
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
    }
}

void S2Plugin::ViewToolbar::showDebugSettings()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto ptr = Spelunky2::get()->get_DebugSettingsPtr();
        if (ptr != 0)
        {
            auto w = new ViewStruct(ptr, Configuration::get()->typeFieldsOfDefaultStruct("DebugSettings"), "Debug Settings");
            auto win = mMDIArea->addSubWindow(w);
            win->setVisible(true);
            win->setAttribute(Qt::WA_DeleteOnClose);
        }
    }
}

void S2Plugin::ViewToolbar::showEntityFactory()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_EntityDB().isValid())
    {
        auto w = new ViewEntityFactory();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showLogger()
{
    auto w = new ViewLogger();
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showSaveStates()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto w = new ViewSaveStates();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::clearLabels()
{
    // (-1) since the full max value causes some overflow(?) and removes all labels, not only the automatic ones
    DbgClearAutoLabelRange(0, std::numeric_limits<duint>::max() - 1);
}

void S2Plugin::ViewToolbar::reloadConfig()
{
    auto windows = mMDIArea->subWindowList();
    for (const auto& window : windows)
    {
        if (qobject_cast<ViewEntities*>(window->widget()) == nullptr)
        {
            window->close();
        }
    }
    Configuration::reload();
}
