#pragma once

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QTreeView>
#include <QWidget>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace S2Plugin
{
    struct MemoryField;

    constexpr char* gsDragDropMemoryField_UID = "uid";
    constexpr char* gsDragDropMemoryField_Address = "addr";
    constexpr char* gsDragDropMemoryField_Type = "type";
    constexpr char* gsDragDropMemoryField_IsPointer = "pointer";
    constexpr char* gsDragDropMemoryField_RefName = "ref";

    struct ColumnFilter
    {
        constexpr ColumnFilter& enable(uint8_t h)
        {
            activeColumns = activeColumns | (1U << h);
            return *this;
        }
        constexpr ColumnFilter& disable(uint8_t h)
        {
            activeColumns = activeColumns & ~(1U << h);
            return *this;
        }
        constexpr bool test(uint8_t h) const
        {
            return (activeColumns & (1U << h)) != 0;
        }

      private:
        uint16_t activeColumns{0xFFFF};
    };

    class TreeViewMemoryFields : public QTreeView
    {
        Q_OBJECT
      public:
        explicit TreeViewMemoryFields(QWidget* parent = nullptr);

        void addMemoryFields(const std::vector<MemoryField>& fields, const std::string& mainName, uintptr_t structAddr, size_t initialDelta = 0, uint8_t deltaPrefixCount = 0,
                             QStandardItem* parent = nullptr);
        QStandardItem* addMemoryField(const MemoryField& field, const std::string& fieldNameOverride, uintptr_t memoryAddress, size_t delta, uint8_t deltaPrefixCount = 0,
                                      QStandardItem* parent = nullptr);
        void clear();
        void updateTableHeader(bool restoreColumnWidths = true);
        void setEnableChangeHighlighting(bool b) noexcept
        {
            mEnableChangeHighlighting = b;
        }

        void updateTree(uintptr_t newAddr, uintptr_t newComparisonAddr = 0, bool initial = false);
        void updateRow(int row, std::optional<uintptr_t> newAddr = std::nullopt, std::optional<uintptr_t> newAddrComparison = std::nullopt, QStandardItem* parent = nullptr,
                       bool disableChangeHighlightingForField = false);

        ColumnFilter activeColumns;
        void labelAll(std::string_view prefix);
        void expandLast();

      public slots:
        void labelAll() // for the slots so we don't corrupt the parameters
        {
            labelAll({});
        }
        void updateTree()
        {
            updateTree(0, 0, false);
        }

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void startDrag(Qt::DropActions supportedActions) override;

      signals:
        void memoryFieldValueUpdated(int row, QStandardItem* parrent);
        void levelGenRoomsPointerClicked();
        void offsetDropped(uintptr_t offset);

      private slots:
        void cellClicked(const QModelIndex& index);

      private:
        QStandardItemModel* mModel;
        std::array<uint32_t, 9> mSavedColumnWidths = {0};
        bool mEnableChangeHighlighting = true;
    };
} // namespace S2Plugin
