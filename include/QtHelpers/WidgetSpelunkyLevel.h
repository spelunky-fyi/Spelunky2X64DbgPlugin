#pragma once

#include "Data/Entity.h"
#include <QBrush>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace S2Plugin
{
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

      protected:
        void paintEvent(QPaintEvent* event) override;

      private:
        uintptr_t mMainEntityAddr{0};

        bool mPaintFloors{false};
        QBrush mFloorColor;
        uint mLevelWidth{0};
        uint mLevelHeight{0};

        std::pair<uintptr_t, uintptr_t> mMaskMapAddr;
        std::pair<uintptr_t, uintptr_t> mGridEntitiesAddr;

        uint32_t mEntityMasksToPaint{0};
        std::array<QBrush, 15> mEntityMaskColors;
        std::vector<std::pair<Entity, QBrush>> mEntitiesToPaint;

        static constexpr uint8_t msLevelMaxHeight = 125;
        static constexpr uint8_t msLevelMaxWidth = 85;
        static constexpr uint8_t msMarginVer = 1;
        static constexpr uint8_t msMarginHor = 1;
        static constexpr uint8_t msScaleFactor = 7;
    };

} // namespace S2Plugin
