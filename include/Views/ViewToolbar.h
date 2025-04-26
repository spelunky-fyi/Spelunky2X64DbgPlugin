#pragma once

#include <QDockWidget>
#include <QMdiArea>
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewEntityDB;
    class ViewParticleDB;
    class ViewVirtualTable;
    class ViewTextureDB;
    class ViewCharacterDB;

    class ViewToolbar : public QDockWidget
    {
        Q_OBJECT
      public:
        ViewToolbar(QMdiArea* mdiArea, QWidget* parent = nullptr);
        void showEntity(uintptr_t address);
        void showState(uintptr_t statePtr);
        void showStdVector(uintptr_t address, const std::string& typeName);
        void showStdMap(uintptr_t address, const std::string& keyTypeName, const std::string& valueTypeName);
        void showStdUnorderedMap(uintptr_t address, const std::string& keyTypeName, const std::string& valueTypeName);
        void showVirtualFunctions(uintptr_t address, const std::string& typeName);
        void showJournalPage(uintptr_t address);
        void showLevelGen(uintptr_t address);
        void showLiquidPhysics(uintptr_t address);
        void showArray(uintptr_t address, std::string name, std::string arrayTypeName, size_t length);
        void showMatrix(uintptr_t address, std::string name, std::string arrayTypeName, size_t rows, size_t columns);
        void showEntityList(uintptr_t address);
        void showStdList(uintptr_t address, std::string typeName, bool oldType = false);
        void showSaveGame(uintptr_t address);

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        ViewTextureDB* showTextureDB();
        void showStringsTable();
        ViewCharacterDB* showCharacterDB();
        void showMainThreadState();
        void showGameManager();
        void showMainThreadLevelGen();
        void showMainThreadLiquidPhysics();
        void showEntities();
        ViewVirtualTable* showVirtualTableLookup();
        void showMainThreadSaveGame();
        void showLogger();
        void showOnline();
        void showSaveStates();
        void showGameAPI();
        void showHud();
        void showEntityFactory();
        void showDebugSettings();

      private slots:
        void clearLabels();
        void reloadConfig();

      private:
        QMdiArea* mMDIArea;
        friend struct QtPluginStruct;
    };
} // namespace S2Plugin
