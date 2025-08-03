#pragma once

#include <QWidget>
#include <cstdint>
#include <string>
#include <unordered_map>

class QTabWidget;
class QMenu;

namespace S2Plugin
{
    class TreeViewMemoryFields;
    class WidgetSpelunkyRooms;

    class ViewLevelGen : public QWidget
    {
        Q_OBJECT
      public:
        ViewLevelGen(uintptr_t address, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshLevelGen();
        void label();
        void levelGenRoomsPointerClicked();
        void viewContextMenu(QMenu* menu);

      private:
        QTabWidget* mMainTabWidget;

        // TAB DATA
        TreeViewMemoryFields* mMainTreeView;
        uintptr_t mLevelGenPtr;

        // TAB LEVEL
        std::unordered_map<std::string, WidgetSpelunkyRooms*> mRoomsWidgets; // field_name -> widget*
        bool mUseEnumNames{false};
    };
} // namespace S2Plugin
