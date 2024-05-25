#include "Views/ViewToolbar.h"

#include "Configuration.h"
#include "Spelunky2.h"
#include "Views/ViewCharacterDB.h"
#include "Views/ViewEntities.h"
#include "Views/ViewEntity.h"
#include "Views/ViewEntityDB.h"
#include "Views/ViewJournalPage.h"
#include "Views/ViewLevelGen.h"
#include "Views/ViewLogger.h"
#include "Views/ViewParticleDB.h"
#include "Views/ViewStdMap.h"
#include "Views/ViewStdVector.h"
#include "Views/ViewStringsTable.h"
#include "Views/ViewStruct.h"
#include "Views/ViewTextureDB.h"
#include "Views/ViewThreads.h"
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

    setTitleBarWidget(new QWidget(this));

    auto btnEntityDB = new QPushButton(this);
    btnEntityDB->setText("Entity DB");
    mainLayout->addWidget(btnEntityDB);
    QObject::connect(btnEntityDB, &QPushButton::clicked, this, &ViewToolbar::showEntityDB);

    auto btnTextureDB = new QPushButton(this);
    btnTextureDB->setText("Texture DB");
    mainLayout->addWidget(btnTextureDB);
    QObject::connect(btnTextureDB, &QPushButton::clicked, this, &ViewToolbar::showTextureDB);

    auto btnParticleDB = new QPushButton(this);
    btnParticleDB->setText("Particle DB");
    mainLayout->addWidget(btnParticleDB);
    QObject::connect(btnParticleDB, &QPushButton::clicked, this, &ViewToolbar::showParticleDB);

    auto btnStringsTable = new QPushButton(this);
    btnStringsTable->setText("Strings DB");
    mainLayout->addWidget(btnStringsTable);
    QObject::connect(btnStringsTable, &QPushButton::clicked, this, &ViewToolbar::showStringsTable);

    auto btnCharacterDB = new QPushButton(this);
    btnCharacterDB->setText("Character DB");
    mainLayout->addWidget(btnCharacterDB);
    QObject::connect(btnCharacterDB, &QPushButton::clicked, this, &ViewToolbar::showCharacterDB);

    auto divider = new QFrame(this);
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(divider);

    auto btnState = new QPushButton(this);
    btnState->setText("State");
    mainLayout->addWidget(btnState);
    QObject::connect(btnState, &QPushButton::clicked, this, &ViewToolbar::showMainThreadState);

    auto btnEntities = new QPushButton(this);
    btnEntities->setText("Entities");
    mainLayout->addWidget(btnEntities);
    QObject::connect(btnEntities, &QPushButton::clicked, this, &ViewToolbar::showEntities);

    auto btnLevelGen = new QPushButton(this);
    btnLevelGen->setText("LevelGen");
    mainLayout->addWidget(btnLevelGen);
    QObject::connect(btnLevelGen, &QPushButton::clicked, this, &ViewToolbar::showMainThreadLevelGen);

    auto btnGameManager = new QPushButton(this);
    btnGameManager->setText("GameManager");
    mainLayout->addWidget(btnGameManager);
    QObject::connect(btnGameManager, &QPushButton::clicked, this, &ViewToolbar::showGameManager);

    auto btnSaveGame = new QPushButton(this);
    btnSaveGame->setText("SaveGame");
    mainLayout->addWidget(btnSaveGame);
    QObject::connect(btnSaveGame, &QPushButton::clicked, this, &ViewToolbar::showSaveGame);

    auto btnOnline = new QPushButton(this);
    btnOnline->setText("Online");
    mainLayout->addWidget(btnOnline);
    QObject::connect(btnOnline, &QPushButton::clicked, this, &ViewToolbar::showOnline);

    auto btnVirtualTable = new QPushButton(this);
    btnVirtualTable->setText("Virtual Table");
    mainLayout->addWidget(btnVirtualTable);
    QObject::connect(btnVirtualTable, &QPushButton::clicked, this, &ViewToolbar::showVirtualTableLookup);

    auto divider2 = new QFrame(this);
    divider2->setFrameShape(QFrame::HLine);
    divider2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(divider2);

    auto btnLogger = new QPushButton(this);
    btnLogger->setText("Logger");
    mainLayout->addWidget(btnLogger);
    QObject::connect(btnLogger, &QPushButton::clicked, this, &ViewToolbar::showLogger);

    auto btnThreads = new QPushButton(this);
    btnThreads->setText("Threads");
    mainLayout->addWidget(btnThreads);
    QObject::connect(btnThreads, &QPushButton::clicked, this, &ViewToolbar::showThreads);

    mainLayout->addStretch();

    auto btnClearLabels = new QPushButton(this);
    btnClearLabels->setText("Clear labels");
    mainLayout->addWidget(btnClearLabels);
    QObject::connect(btnClearLabels, &QPushButton::clicked, this, &ViewToolbar::clearLabels);

    auto btnReloadConfig = new QPushButton(this);
    btnReloadConfig->setText("Reload JSON");
    mainLayout->addWidget(btnReloadConfig);
    QObject::connect(btnReloadConfig, &QPushButton::clicked, this, &ViewToolbar::reloadConfig);
}

void S2Plugin::ViewToolbar::showVirtualFunctions(uintptr_t address, const std::string& typeName)
{
    auto w = new ViewVirtualFunctions(typeName, address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdVector(uintptr_t address, const std::string& typeName)
{
    auto w = new ViewStdVector(typeName, address);
    auto win = mMDIArea->addSubWindow(w);
    win->setVisible(true);
    win->setAttribute(Qt::WA_DeleteOnClose);
}

void S2Plugin::ViewToolbar::showStdMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName)
{
    auto w = new ViewStdMap(keytypeName, valuetypeName, address);
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

void S2Plugin::ViewToolbar::showEntity(uintptr_t address)
{
    auto w = new ViewEntity(address);
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
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_StringsTable().isValid() && Spelunky2::get()->get_CharacterDB().isValid())
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
        auto statePtr = Spelunky2::get()->get_StatePtr();
        if (statePtr != 0)
            showState(statePtr);
    }
}

void S2Plugin::ViewToolbar::showMainThreadLevelGen()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto levelGenPtr = Spelunky2::get()->get_LevelGenPtr();
        if (levelGenPtr != 0)
            showLevelGen(levelGenPtr);
    }
}

void S2Plugin::ViewToolbar::showGameManager()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_GameManagerPtr() != 0)
    {
        auto w = new ViewStruct(Spelunky2::get()->get_GameManagerPtr(), Configuration::get()->typeFields(MemoryFieldType::GameManager), "GameManager");
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
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
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_StringsTable().isValid())
    {
        auto w = new ViewStringsTable();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showOnline()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_OnlinePtr() != 0)
    {
        auto w = new ViewStruct(Spelunky2::get()->get_OnlinePtr(), Configuration::get()->typeFields(MemoryFieldType::Online), "Online");
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showEntities()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_EntityDB().isValid())
    {
        auto w = new ViewEntities();
        auto win = mMDIArea->addSubWindow(w);
        win->setVisible(true);
        win->setAttribute(Qt::WA_DeleteOnClose);
    }
}

void S2Plugin::ViewToolbar::showSaveGame()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded() && Spelunky2::get()->get_SaveDataPtr() != 0)
    {
        auto w = new ViewStruct(Spelunky2::get()->get_SaveDataPtr(), Configuration::get()->typeFields(MemoryFieldType::SaveGame), "SaveGame");
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

void S2Plugin::ViewToolbar::showThreads()
{
    if (Spelunky2::is_loaded() && Configuration::is_loaded())
    {
        auto w = new ViewThreads();
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
