#include "QtHelpers/WidgetSpelunkyLevel.h"
#include "Configuration.h"
#include "Data/StdMap.h"
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QPainter>

S2Plugin::WidgetSpelunkyLevel::WidgetSpelunkyLevel(uintptr_t main_entity, QWidget* parent) : QWidget(parent), mMainEntityAddr(main_entity)
{
    auto stateptr = Spelunky2::get()->get_StatePtr();
    mMaskMapAddr.first = Configuration::get()->offsetForField(MemoryFieldType::State, "layer0.entities_by_mask", stateptr);
    mMaskMapAddr.second = Configuration::get()->offsetForField(MemoryFieldType::State, "layer1.entities_by_mask", stateptr);
    mGridEntitiesAddr.first = Configuration::get()->offsetForField(MemoryFieldType::State, "layer0.grid_entities_begin", stateptr);
    mGridEntitiesAddr.second = Configuration::get()->offsetForField(MemoryFieldType::State, "layer1.grid_entities_begin", stateptr);

    auto offset = Configuration::get()->offsetForField(MemoryFieldType::State, "level_width_rooms", stateptr);
    mLevelWidth = Script::Memory::ReadDword(offset) * 10;
    mLevelHeight = Script::Memory::ReadDword(offset + 4) * 8;

    if (mLevelWidth == 0)
    {
        mLevelWidth = msLevelMaxWidth;
        mLevelHeight = msLevelMaxHeight;
    }
    else
    {
        // add border size
        mLevelWidth += 5;
        mLevelHeight += 5;
    }
}

void S2Plugin::WidgetSpelunkyLevel::paintEvent(QPaintEvent* event)
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
        // painting floors is quite expensive, any optimisations as always welcome (like maybe use provided event to not draw obstructed parts of the level?)
        painter.setBrush(mFloorColor);
        auto gridAddr = layerToDraw == 0 ? mGridEntitiesAddr.first : mGridEntitiesAddr.second;
        // y: 0-125, x: 0-85
        // note: the y = 125 is at the top of a level and the level is build from the top
        for (uint8_t y = msLevelMaxHeight - mLevelHeight; y < 126; ++y)
        {
            for (uint8_t x = 0; x <= mLevelWidth; ++x)
            {
                if (Script::Memory::ReadQword(gridAddr + y * (86 * sizeof(uintptr_t)) + x * sizeof(uintptr_t)) != 0)
                    painter.drawRect(QRectF(msMarginHor + x, msMarginVer + msLevelMaxHeight - y, 1.0, 1.0));
            }
        }
    }
    if (mEntityMasksToPaint != 0)
    {
        StdMap<uint32_t, size_t> maskMap{layerToDraw == 0 ? mMaskMapAddr.first : mMaskMapAddr.second};
        for (uint8_t bit_number = 0; bit_number < mEntityMaskColors.size(); ++bit_number)
        {
            if ((mEntityMasksToPaint >> bit_number) & 1)
            {
                auto itr = maskMap.find(1u << bit_number);
                if (itr != maskMap.end())
                {
                    painter.setBrush(mEntityMaskColors[bit_number]);
                    // TODO: change to proper struct when done
                    auto ent_list = itr.value_ptr();
                    auto pointers = Script::Memory::ReadQword(ent_list);
                    auto list_count = Script::Memory::ReadDword(ent_list + 20);
                    for (uint idx = 0; idx < list_count; idx++)
                    {
                        Entity ent{Script::Memory::ReadQword(pointers + idx * sizeof(uintptr_t))};
                        auto [entityX, entityY] = ent.abs_position();
                        painter.drawRect(QRectF(msMarginHor + entityX, msMarginVer + msLevelMaxHeight - entityY, 1.0, 1.0));
                    }
                }
            }
        }
    }

    for (auto& [entity, color] : mEntitiesToPaint)
    {
        auto [entityX, entityY] = entity.abs_position();
        painter.setBrush(color);
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
    update();
}

void S2Plugin::WidgetSpelunkyLevel::clearPaintedEntity(uintptr_t addr)
{
    for (auto cur = mEntitiesToPaint.begin(); cur < mEntitiesToPaint.end(); ++cur)
    {
        if (cur->first.ptr() == addr)
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
