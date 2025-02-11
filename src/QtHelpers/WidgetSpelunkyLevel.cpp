#include "QtHelpers/WidgetSpelunkyLevel.h"

#include "Configuration.h"
#include "Data/EntityList.h"
#include "Data/StdMap.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QPainter>

S2Plugin::WidgetSpelunkyLevel::WidgetSpelunkyLevel(uintptr_t main_entity, QWidget* parent) : QWidget(parent), mMainEntityAddr(main_entity)
{
    auto stateptr = Spelunky2::get()->get_StatePtr(false);
    mMaskMapAddr.first = Configuration::get()->offsetForField(MemoryFieldType::State, "layer0.entities_by_mask", stateptr);
    mMaskMapAddr.second = Configuration::get()->offsetForField(MemoryFieldType::State, "layer1.entities_by_mask", stateptr);
    mGridEntitiesAddr.first = Configuration::get()->offsetForField(MemoryFieldType::State, "layer0.grid_entities", stateptr);
    mGridEntitiesAddr.second = Configuration::get()->offsetForField(MemoryFieldType::State, "layer1.grid_entities", stateptr);

    // auto offset = Configuration::get()->offsetForField(MemoryFieldType::State, "level_width_rooms", stateptr);
    // mLevelWidth = Script::Memory::ReadDword(offset) * 10;
    // mLevelHeight = Script::Memory::ReadDword(offset + 4) * 8;

    // if (mLevelWidth == 0)
    //{
    //     mLevelWidth = msLevelMaxWidth;
    //     mLevelHeight = msLevelMaxHeight;
    // }
    // else
    //{
    //     // add border size
    //     mLevelWidth += 5;
    //     mLevelHeight += 5;

    //    // limit just in case
    //    if (mLevelWidth > msLevelMaxWidth)
    //        mLevelWidth = msLevelMaxWidth;
    //    if (mLevelHeight > msLevelMaxHeight)
    //        mLevelHeight = msLevelMaxHeight;
    //}
}

void S2Plugin::WidgetSpelunkyLevel::paintEvent(QPaintEvent*)
{
    auto painter = QPainter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    uint8_t layerToDraw = Entity{mMainEntityAddr}.layer();
    if (layerToDraw > 1)
        return;

    // DRAW ENTITY BLOCKS
    painter.save();
    painter.scale(msScaleFactor, msScaleFactor);
    painter.setPen(Qt::transparent);
    if (mPaintFloors)
    {
        // painting floors is quite expensive, any optimisations are always welcome (like maybe use provided event to not draw obstructed parts of the level view?)
        painter.setBrush(mFloorColor);
        // y: 0-125, x: 0-85
        // note: the y = 125 is at the top of a level and the level is build from the top
        for (uint8_t y = 0; y < msLevelMaxHeight + 1; ++y)
            for (uint8_t x = 0; x <= msLevelMaxWidth + 1; ++x)
                if (mLevelFloors[y][x] != 0)
                    painter.drawRect(QRectF(msMarginHor + x, msMarginVer + msLevelMaxHeight - y, 1.0, 1.0));
    }
    if (mEntityMasksToPaint != 0)
    {
        for (uint8_t bit_number = 0; bit_number < mEntityMaskColors.size(); ++bit_number)
        {
            if ((mEntityMasksToPaint >> bit_number) & 1)
            {
                painter.setBrush(mEntityMaskColors[bit_number]);
                for (auto& [posX, posY] : mEntitiesMaskCoordinates[bit_number])
                    painter.drawRect(QRectF(msMarginHor + posX, msMarginVer + msLevelMaxHeight - posY, 1.0, 1.0));
            }
        }
    }

    for (auto& entity : mEntitiesToPaint)
    {
        const auto& [entityX, entityY] = entity.pos;
        painter.setBrush(entity.color);
        painter.drawRect(QRectF(msMarginHor + entityX, msMarginVer + msLevelMaxHeight - entityY, 1.0, 1.0));
    }

    painter.restore();

    // DRAW BORDER
    painter.setPen(QPen(Qt::black, 1.0));
    painter.setBrush(Qt::transparent);
    painter.drawRect(QRectF(msMarginHor * msScaleFactor, msMarginVer * msScaleFactor, ((msLevelMaxWidth + msMarginHor) * msScaleFactor) - .5, ((msLevelMaxHeight + msMarginVer) * msScaleFactor) - .5));
}

void S2Plugin::WidgetSpelunkyLevel::paintEntity(uintptr_t addr, const QColor& color)
{
    mEntitiesToPaint.emplace_back(addr, color);
}

void S2Plugin::WidgetSpelunkyLevel::paintFloor(const QColor& color)
{
    mPaintFloors = true;
    mFloorColor = color;
}

void S2Plugin::WidgetSpelunkyLevel::paintEntityMask(uint32_t entityMask, const QColor& color)
{
    mEntityMasksToPaint |= entityMask;

    for (uint8_t bit_number = 0; bit_number < mEntityMaskColors.size(); ++bit_number)
        if ((entityMask >> bit_number) & 1)
            mEntityMaskColors[bit_number] = color;
}

void S2Plugin::WidgetSpelunkyLevel::clearAllPaintedEntities()
{
    mEntitiesToPaint.clear();
    mEntityMasksToPaint = 0;
    mPaintFloors = false;
    for (uint8_t bit_number = 0; bit_number < mEntityMaskColors.size(); ++bit_number)
        mEntitiesMaskCoordinates[bit_number].clear();

    update();
}

void S2Plugin::WidgetSpelunkyLevel::clearPaintedEntity(uintptr_t addr)
{
    for (auto cur = mEntitiesToPaint.begin(); cur < mEntitiesToPaint.end(); ++cur)
    {
        if (cur->ent.ptr() == addr)
        {
            mEntitiesToPaint.erase(cur);
            break;
        }
    }
}

QSize S2Plugin::WidgetSpelunkyLevel::minimumSizeHint() const
{
    auto width = msScaleFactor * ((msMarginHor * 2) + msLevelMaxWidth);
    auto height = msScaleFactor * ((msMarginVer * 2) + msLevelMaxHeight);
    return QSize(width, height);
}

QSize S2Plugin::WidgetSpelunkyLevel::sizeHint() const
{
    return minimumSizeHint();
}

void S2Plugin::WidgetSpelunkyLevel::updateLevel()
{
    uint8_t layerToDraw = Entity{mMainEntityAddr}.layer();
    if (mPaintFloors)
    {
        auto gridAddr = layerToDraw == 0 ? mGridEntitiesAddr.first : mGridEntitiesAddr.second;
        // Maybe don't read the whole array?
        constexpr auto dataSize = (msLevelMaxHeight + 1) * ((msLevelMaxWidth + 1) * sizeof(uintptr_t));
        Script::Memory::Read(gridAddr, &mLevelFloors, dataSize, nullptr);
    }

    for (auto& entity : mEntitiesToPaint)
        entity.pos = entity.ent.abs_position();

    if (mEntityMasksToPaint != 0)
    {
        StdMap<uint32_t, size_t> maskMap{layerToDraw == 0 ? mMaskMapAddr.first : mMaskMapAddr.second};
        for (auto [key, value_ptr] : maskMap)
        {
            uint8_t bit_number = std::log2(key);
            mEntitiesMaskCoordinates[bit_number].clear();
            if ((mEntityMasksToPaint & key) != 0)
            {
                EntityList entityList{value_ptr};
                mEntitiesMaskCoordinates[bit_number].reserve(entityList.size());
                std::vector<uintptr_t> entities = entityList.getAllEntities();

                for (auto entityAddr : entities)
                {
                    if (entityAddr == 0)
                        continue;

                    Entity ent{entityAddr};
                    mEntitiesMaskCoordinates[bit_number].emplace_back(ent.abs_position());
                }
            }
        }
    }
    update();
}
