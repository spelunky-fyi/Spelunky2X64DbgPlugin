#include "QtHelpers/WidgetSpelunkyRooms.h"

#include "Configuration.h"
#include "QtHelpers/WidgetMemoryView.h" // for ToolTipRect
#include "Spelunky2.h"
#include "pluginmain.h"
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QToolTip>
#include <array>

static const int gsMarginHor = 10;
static const int gsMarginVer = 5;
static constexpr size_t gsBufferSize = 8 * 16 * 2;     // 8x16 rooms * 2 bytes per room
static constexpr size_t gsHalfBufferSize = 8 * 16 * 1; // 8x16 rooms * 1 byte/bool per room

S2Plugin::WidgetSpelunkyRooms::WidgetSpelunkyRooms(const std::string& fieldName, QWidget* parent) : QWidget(parent), mFieldName(QString::fromStdString(fieldName))
{
    auto font = QFont("Courier", 11);
    mTextAdvance = QFontMetrics(font).size(Qt::TextSingleLine, "00");
    mSpaceAdvance = QFontMetrics(font).size(Qt::TextSingleLine, " ").width();
    setMouseTracking(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed));
}

void S2Plugin::WidgetSpelunkyRooms::paintEvent(QPaintEvent*)
{
    const static auto font = QFont("Courier", 11);
    auto config = Configuration::get();
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter.setFont(font);
    {
        auto rect = QRectF(QPointF(0, 0), sizeHint());
        rect.adjust(0, 0, -0.5, -0.5);
        painter.setBrush(Qt::white);
        painter.setPen(QPen(Qt::darkGray, 1));
        painter.drawRect(rect);
    }
    mToolTipRects.clear();
    painter.setBrush(Qt::black);
    int x = gsMarginHor;
    int y = gsMarginVer + mTextAdvance.height();

    painter.drawText(x, y, mFieldName);
    y += mTextAdvance.height() + gsMarginVer;
    if (mOffset != 0)
    {
        auto bufferSize = mIsMetaData ? gsHalfBufferSize : gsBufferSize;
        auto buffer = std::array<uint8_t, gsBufferSize>();
        Script::Memory::Read(mOffset, buffer.data(), bufferSize, nullptr);

        for (size_t counter = 0; counter < bufferSize; ++counter)
        {
            if (mIsMetaData)
            {
                if (buffer.at(counter) == 1)
                {
                    painter.setPen(QPen(Qt::white, 1));
                    painter.setBrush(Qt::black);
                    painter.drawRect(QRect(x, y - mTextAdvance.height() + 5, mTextAdvance.width(), mTextAdvance.height() - 2));
                }
                else
                {
                    painter.setPen(QPen(Qt::black, 1));
                }
            }
            else
            {
                RoomCode currentRoomCode{0, "", QColor{}};
                if (counter % 2 == 0)
                {
                    currentRoomCode = config->roomCodeForID(buffer.at(counter));
                    painter.setPen(Qt::transparent);
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, 2 * mTextAdvance.width() + mSpaceAdvance, mTextAdvance.height() - 2);
                    painter.setBrush(currentRoomCode.color);
                    painter.drawRoundedRect(rect, 4.0, 4.0);
                    mToolTipRects.emplace_back(ToolTipRect{rect, QString::fromStdString(currentRoomCode.name)});
                }

                if (currentRoomCode.id == 0 || currentRoomCode.id == 9)
                {
                    painter.setPen(QPen(Qt::lightGray, 1));
                }
                else
                {
                    painter.setPen(QPen(Qt::black, 1));
                }
            }

            auto str = QString("%1").arg(buffer.at(counter), 2, 16, QChar('0'));
            painter.drawText(x, y, str);
            x += mTextAdvance.width() + mSpaceAdvance;

            auto cutoff = 16;
            if (mIsMetaData)
            {
                cutoff = 8;
            }

            if ((counter + 1) % cutoff == 0)
            {
                y += mTextAdvance.height();
                x = gsMarginHor;
            }
        }

        // draw level dimensions
        if (auto statePtr = Spelunky2::get()->get_StatePtr(true); statePtr != 0)
        {
            uintptr_t offsetWidth = config->offsetForField(config->typeFields(MemoryFieldType::State), "level_width_rooms", statePtr);
            uintptr_t offsetHeight = config->offsetForField(config->typeFields(MemoryFieldType::State), "level_height_rooms", statePtr);
            int levelWidth = static_cast<int>(Script::Memory::ReadDword(offsetWidth));
            int levelHeight = static_cast<int>(Script::Memory::ReadDword(offsetHeight));
            int borderX = gsMarginHor;
            int borderY = (2 * gsMarginVer) + mTextAdvance.height() + 4;
            int borderWidth, borderHeight;
            if (mIsMetaData)
            {
                borderWidth = (levelWidth * (mTextAdvance.width() + mSpaceAdvance)) - mSpaceAdvance;
                borderHeight = levelHeight * mTextAdvance.height();
            }
            else
            {
                borderWidth = (levelWidth * (2 * (mTextAdvance.width() + mSpaceAdvance))) - mSpaceAdvance;
                borderHeight = levelHeight * mTextAdvance.height();
            }
            auto border = QRect(borderX, borderY, borderWidth, borderHeight);
            border.adjust(-2, -2, +2, +2);
            painter.setPen(QPen(Qt::blue, 1));
            painter.setBrush(Qt::transparent);
            painter.drawRect(border);
        }
    }
}

QSize S2Plugin::WidgetSpelunkyRooms::sizeHint() const
{
    return minimumSizeHint();
}

QSize S2Plugin::WidgetSpelunkyRooms::minimumSizeHint() const
{
    auto bufferSize = mIsMetaData ? gsHalfBufferSize : gsBufferSize;
    auto cutoff = 16;
    if (mIsMetaData)
    {
        cutoff = 8;
    }

    int totalWidth = ((mTextAdvance.width() + mSpaceAdvance) * cutoff) + (gsMarginHor * 2) - mSpaceAdvance;
    int totalHeight = gsMarginVer + mTextAdvance.height() + (mTextAdvance.height() * static_cast<int>(std::ceil(static_cast<double>(bufferSize) / static_cast<double>(cutoff)))) + (gsMarginVer * 2) +
                      mTextAdvance.height();

    return QSize(totalWidth, totalHeight);
}

void S2Plugin::WidgetSpelunkyRooms::setOffset(size_t offset)
{
    mOffset = offset;
    update();
    updateGeometry();
    adjustSize();
}

void S2Plugin::WidgetSpelunkyRooms::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = event->pos();
    uint32_t idx = 0;
    for (const auto& ttr : mToolTipRects)
    {
        if (ttr.rect.contains(pos))
        {
            if (idx != mCurrentToolTip)
            {
                mCurrentToolTip = idx;
                QToolTip::showText({}, {});
            }
            QToolTip::showText(mapToGlobal(pos), ttr.tooltip);
            return;
        }
        ++idx;
    }
}

void S2Plugin::WidgetSpelunkyRooms::setIsMetaData()
{
    mIsMetaData = true;
}
