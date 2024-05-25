#pragma once

#include "Data/Entity.h"
#include <QBrush>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>
#include <array>
#include <bitset>
#include <cstdint>
#include <utility>
#include <vector>

namespace S2Plugin
{
    class EntityToPaint
    {
      public:
        EntityToPaint() = default;
        EntityToPaint(uintptr_t addr, QBrush col) : ent(addr), color(col){};

      protected:
        Entity ent;
        QBrush color;
        std::pair<float, float> pos{0, 0};
        friend class WidgetSpelunkyLevel;
    };

    class WidgetSpelunkyLevel : public QWidget
    {
        Q_OBJECT

      public:
        // main_entity used to get the layer
        explicit WidgetSpelunkyLevel(uintptr_t main_entity, QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void paintEntityMask(uint32_t entityMask, const QColor& color);
        void paintEntity(uintptr_t entityUID, const QColor& color);
        void paintFloor(const QColor& color);
        void clearAllPaintedEntities();
        void clearPaintedEntity(uintptr_t addr);
        void updateLevel();

      protected:
        void paintEvent(QPaintEvent* event) override;

      private:
        uintptr_t mMainEntityAddr{0};

        QBrush mFloorColor;
        bool mPaintFloors{false};
        // uint8_t mLevelWidth{0};
        // uint8_t mLevelHeight{0};

        std::pair<uintptr_t, uintptr_t> mMaskMapAddr;
        std::pair<uintptr_t, uintptr_t> mGridEntitiesAddr;

        uint32_t mEntityMasksToPaint{0};
        std::array<QBrush, 15> mEntityMaskColors;
        std::array<std::vector<std::pair<float, float>>, 15> mEntitiesMaskCoordinates;
        std::vector<EntityToPaint> mEntitiesToPaint;

        static constexpr uint8_t msLevelMaxHeight = 125;
        static constexpr uint8_t msLevelMaxWidth = 85;
        static constexpr uint8_t msMarginVer = 1;
        static constexpr uint8_t msMarginHor = 1;
        static constexpr uint8_t msScaleFactor = 7;

        uintptr_t mLevelFloors[msLevelMaxHeight + 1][msLevelMaxWidth + 1] = {};
    };

} // namespace S2Plugin
