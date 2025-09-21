#pragma once

#include "Configuration.h"
#include <QTreeView>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class QStandardItemModel;
class QStandardItem;

namespace S2Plugin
{
    constexpr const char* gsDragDropMemoryField_UID = "uid";
    constexpr const char* gsDragDropMemoryField_Address = "addr";
    constexpr const char* gsDragDropMemoryField_Type = "type";
    constexpr const char* gsDragDropMemoryField_IsPointer = "pointer";
    constexpr const char* gsDragDropMemoryField_RefName = "ref";

    struct ColumnFilter
    {
        explicit constexpr ColumnFilter(uint16_t a) : mActiveColumns(a){};
        explicit constexpr ColumnFilter(){};
        constexpr ColumnFilter& enable(uint8_t h) noexcept
        {
            mActiveColumns = mActiveColumns | (1U << h);
            return *this;
        }
        constexpr ColumnFilter& disable(uint8_t h) noexcept
        {
            mActiveColumns = mActiveColumns & ~(1U << h);
            return *this;
        }
        constexpr bool test(uint8_t h) const noexcept
        {
            return (mActiveColumns & (1U << h)) != 0;
        }
        constexpr ColumnFilter& flip(uint8_t h)
        {
            mActiveColumns = mActiveColumns ^ (1U << h);
            return *this;
        }

      private:
        uint16_t mActiveColumns{0xFFFF};
        ColumnFilter(const ColumnFilter&) = delete;
        ColumnFilter& operator=(const ColumnFilter&) = delete;
        ColumnFilter& operator=(const ColumnFilter&&) = delete;
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
        void setEnableTopBranchDrawing(bool b) noexcept
        {
            mDrawTopBranch = b;
        }
        void updateTree(uintptr_t newAddr, uintptr_t newComparisonAddr = 0, bool initial = false);
        void updateRow(int row, std::optional<uintptr_t> newAddr = std::nullopt, std::optional<uintptr_t> newAddrComparison = std::nullopt, QStandardItem* parent = nullptr,
                       bool disableChangeHighlightingForField = false);
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
        void cellClicked(const QModelIndex& index);

      private slots:
        void headerClicked(/* int logicalIndex */);

      protected:
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dragMoveEvent(QDragMoveEvent* event) override;
        void dropEvent(QDropEvent* event) override;
        void startDrag(Qt::DropActions supportedActions) override;
        void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void scrollContentsBy(int dx, int dy) override;
      signals:
        void memoryFieldValueUpdated(int row, QStandardItem* parent);
        void levelGenRoomsPointerClicked();
        void offsetDropped(uintptr_t offset);
        void onContextMenu(QMenu* menu);

      private:
        bool isItemClickable(const QModelIndex& index) const;

      public:
        ColumnFilter mActiveColumns;

      private:
        bool mEnableChangeHighlighting = true;
        bool mDrawTopBranch = true;
        std::array<int, 9> mSavedColumnWidths = {};
        QStandardItemModel* mModel;
    };
} // namespace S2Plugin
