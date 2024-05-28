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
        void showStdMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName);
        void showVirtualFunctions(uintptr_t address, const std::string& typeName);
        void showJournalPage(uintptr_t address);
        void showLevelGen(uintptr_t address);
        void showArray(uintptr_t address, std::string name, std::string arrayTypeName, size_t length);

      public slots:
        ViewEntityDB* showEntityDB();
        ViewParticleDB* showParticleDB();
        ViewTextureDB* showTextureDB();
        void showStringsTable();
        ViewCharacterDB* showCharacterDB();

        void showMainThreadState();
        void showGameManager();
        void showMainThreadLevelGen();
        void showEntities();
        ViewVirtualTable* showVirtualTableLookup();
        void showSaveGame();
        void showLogger();
        void showOnline();
        void showThreads();

      private slots:
        void clearLabels();
        void reloadConfig();

      private:
        QMdiArea* mMDIArea;
        friend struct QtPluginStruct;
    };
} // namespace S2Plugin
