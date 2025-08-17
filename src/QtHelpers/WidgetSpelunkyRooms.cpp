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

constexpr const int gsMarginHor = 10;
constexpr const int gsMarginVer = 5;
constexpr const size_t gsBufferSize = 8 * 16 * 2;     // 8x16 rooms * 2 bytes per room
constexpr const size_t gsHalfBufferSize = 8 * 16 * 1; // 8x16 rooms * 1 byte/bool per room

S2Plugin::WidgetSpelunkyRooms::WidgetSpelunkyRooms(const std::string& fieldName, QWidget* parent) : QWidget(parent), mFieldName(QString::fromStdString(fieldName))
{
    auto font = QFont("Courier", 11);
    mTextAdvance = QFontMetrics(font).size(Qt::TextSingleLine, "00");
    mSpaceAdvance = QFontMetrics(font).size(Qt::TextSingleLine, " ").width();
    setMouseTracking(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed));
}

enum ROOM_TEMPLATES : uint16_t
{
    SIDE = 0,
    PATH_NORMAL = 1,
    PATH_DROP = 2,
    PATH_NOTOP = 3,
    PATH_DROP_NOTOP = 4,
    ENTRANCE = 5,
    ENTRANCE_DROP = 6,
    EXIT = 7,
    EXIT_NOTOP = 8,
    BLACKMARKET_EXIT = 35,
    ANUBIS_ROOM = 38,
    MOAI = 39,
    PEN_ROOM = 45,
    SISTERS_ROOM = 46,
    TUSKFRONTDICESHOP = 47,
    TUSKFRONTDICESHOP_LEFT = 48,
    PALACEOFPLEASURE_1_1 = 53,
    PALACEOFPLEASURE_3_2 = 60,
    MACHINE_BIGROOM_PATH = 102,
    FEELING_FACTORY = 104,
    FEELING_PRISON = 105,
    FEELING_TOMB = 106,
    MACHINE_WIDEROOM_PATH = 107,
    MACHINE_TALLROOM_PATH = 109,
    ALTAR = 115,
    STORAGE_ROOM = 118,
    COG_ALTAR_TOP = 125,
    MOTHERSHIP_EXIT = 127,
    GHISTSHOP = 135,
    EMPRESS_GRAVE = 136,
    OLDHUNTER_KEYROOM = 137,
    OLDHUNTER_REWARDROOM = 138,
};

enum PATH_THROUGH : uint8_t
{
    NONE = 0,
    LEFT = 1,
    RIGHT = 2,
    HORIZONTAL = 3,
    UP = 4,
    DOWN = 8,
    VERTICAL = 12,
};

inline PATH_THROUGH operator|(const PATH_THROUGH x, const PATH_THROUGH y)
{
    return static_cast<PATH_THROUGH>(static_cast<uint8_t>(x) | static_cast<uint8_t>(y));
}
inline PATH_THROUGH& operator^=(PATH_THROUGH& x, const PATH_THROUGH y)
{
    reinterpret_cast<uint8_t&>(x) ^= static_cast<uint8_t>(y);
    return x;
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
        const uint8_t cutoff = mIsMetaData ? 8 : 16;
        Script::Memory::Read(mOffset, buffer.data(), bufferSize, nullptr);
        auto statePtr = Spelunky2::get()->get_StatePtr(true);

        for (size_t counter = 0; counter < bufferSize;)
        {
            ROOM_TEMPLATES roomID = SIDE; // 0
            if (mIsMetaData)
            {
                if (buffer.at(counter) == 1)
                {
                    painter.setPen(QPen(Qt::white, 1));
                    painter.setBrush(Qt::black);
                    painter.drawRect(QRect(x, y - mTextAdvance.height() + 5, mTextAdvance.width(), mTextAdvance.height() - 2));
                }
                else
                    painter.setPen(QPen(Qt::black, 1));
            }
            else
            {
                if (counter % 2 == 0)
                {
                    auto& currentRoomCode = config->roomCodeForID(buffer.at(counter));
                    roomID = (ROOM_TEMPLATES)currentRoomCode.id;
                    painter.setPen(Qt::transparent);
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, 2 * mTextAdvance.width() + mSpaceAdvance, mTextAdvance.height() - 2);
                    painter.setBrush(currentRoomCode.color);
                    painter.drawRoundedRect(rect, 4.0, 4.0);

                    QString name;
                    if (mUseEnum && *mUseEnum)
                        name = QString::fromStdString(currentRoomCode.enumName);
                    else
                        name = QString::fromStdString(currentRoomCode.name);

                    mToolTipRects.emplace_back(ToolTipRect{rect, name});

                    if (currentRoomCode.id == 0 || currentRoomCode.id == 9)
                        painter.setPen(QPen(Qt::lightGray, 1));
                    else
                    {
                        int brightness = 0.2126 * currentRoomCode.color.red() + 0.7152 * currentRoomCode.color.green() + 0.0722 * currentRoomCode.color.blue();
                        if (brightness < 100)
                            painter.setPen(QPen(Qt::white, 1));
                        else
                            painter.setPen(QPen(Qt::black, 1));
                    }
                }
                else
                    painter.setPen(QPen(Qt::lightGray, 1));
            }

            auto str = QString("%1").arg(buffer.at(counter), 2, 16, QChar('0'));
            painter.drawText(x, y, str);

            if (roomID != SIDE && mShowPath && *mShowPath)
            {
                painter.setPen(Qt::transparent);
                painter.setBrush(Qt::GlobalColor::magenta);
                QRect horizontalLine = QRect(x - mSpaceAdvance / 2, y - mTextAdvance.height() + 5, 2 * (mTextAdvance.width() + mSpaceAdvance), mTextAdvance.height() - 2);
                horizontalLine.adjust(0, mTextAdvance.width() / 3, 0, -mTextAdvance.width() / 3);

                QRect verticalLine = QRect(x + mTextAdvance.width() + mSpaceAdvance / 2 - horizontalLine.height() / 2, y - mTextAdvance.height() + 4, horizontalLine.height(), mTextAdvance.height());

                PATH_THROUGH path = PATH_THROUGH::NONE;
                uint8_t themeNumber = Script::Memory::ReadByte(config->offsetForField(config->typeFields(MemoryFieldType::State), "theme", statePtr));
                bool invertPath = false;
                if (themeNumber == 12 || // duat
                    themeNumber == 15 || // EW
                    themeNumber == 9)    // sunken
                    invertPath = true;

                switch (roomID)
                {
                    case OLDHUNTER_KEYROOM:
                    case OLDHUNTER_REWARDROOM:
                    case PEN_ROOM:
                        // case SISTERS_ROOM: // I thoght i got it once on path?
                        painter.setBrush(QColor{245, 140, 2});
                        [[fallthrough]];
                    case PATH_NORMAL:
                    case ENTRANCE:
                    case EXIT:
                    case MACHINE_WIDEROOM_PATH:
                    case BLACKMARKET_EXIT:
                        path = PATH_THROUGH::HORIZONTAL;
                        break;
                    case PALACEOFPLEASURE_1_1:
                        painter.setBrush(QColor{245, 140, 2});
                        [[fallthrough]];
                    case EMPRESS_GRAVE: // always on path
                    case PATH_NOTOP:
                    case EXIT_NOTOP:
                        path = PATH_THROUGH::HORIZONTAL | PATH_THROUGH::UP;
                        break;
                    case PATH_DROP:
                    case ENTRANCE_DROP:
                        path = PATH_THROUGH::HORIZONTAL | PATH_THROUGH::DOWN;
                        break;
                    case PATH_DROP_NOTOP:
                        path = PATH_THROUGH::VERTICAL;
                        break;
                    case ALTAR:
                    {
                        if (themeNumber != 11) // city of gold
                            break;

                        [[fallthrough]];
                    }
                    case COG_ALTAR_TOP:
                    case MOTHERSHIP_EXIT:
                    case STORAGE_ROOM:
                    case MOAI:
                    case GHISTSHOP:
                    case PALACEOFPLEASURE_3_2:
                    case FEELING_PRISON:
                    case FEELING_TOMB:
                    case FEELING_FACTORY:
                        painter.setBrush(QColor{245, 140, 2});
                        [[fallthrough]];
                    case ANUBIS_ROOM: // always on path
                    case MACHINE_TALLROOM_PATH:
                    case MACHINE_BIGROOM_PATH:
                        path = PATH_THROUGH::HORIZONTAL | PATH_THROUGH::VERTICAL;
                        break;
                    case TUSKFRONTDICESHOP: // always on path
                        path = PATH_THROUGH::RIGHT | PATH_THROUGH::VERTICAL;
                        break;
                    case TUSKFRONTDICESHOP_LEFT: // always on path
                        path = PATH_THROUGH::LEFT | PATH_THROUGH::VERTICAL;
                        break;
                }
                if (invertPath && (roomID == EXIT_NOTOP || roomID == ENTRANCE_DROP))
                    path ^= PATH_THROUGH::VERTICAL;

                if (auto result = path & PATH_THROUGH::HORIZONTAL; result)
                {
                    if (result == PATH_THROUGH::LEFT)
                        horizontalLine.setRight(verticalLine.right());
                    else if (result == PATH_THROUGH::RIGHT)
                        horizontalLine.setLeft(verticalLine.left());

                    painter.drawRect(horizontalLine);
                }
                if (auto result = path & PATH_THROUGH::VERTICAL; result)
                {
                    if (result == PATH_THROUGH::UP)
                        verticalLine.setBottom(horizontalLine.bottom());
                    else if (result == PATH_THROUGH::DOWN)
                        verticalLine.setTop(horizontalLine.top());

                    painter.drawRect(verticalLine);
                }
            }

            if ((++counter) % cutoff == 0)
            {
                y += mTextAdvance.height();
                x = gsMarginHor;
            }
            else
                x += mTextAdvance.width() + mSpaceAdvance;
        }

        // draw level dimensions
        if (statePtr != 0)
        {
            uintptr_t offsetWidth = config->offsetForField(config->typeFields(MemoryFieldType::State), "level_width_rooms", statePtr);
            uintptr_t offsetHeight = config->offsetForField(config->typeFields(MemoryFieldType::State), "level_height_rooms", statePtr);
            int levelWidth = static_cast<int>(Script::Memory::ReadDword(offsetWidth));
            int levelHeight = static_cast<int>(Script::Memory::ReadDword(offsetHeight));
            int borderX = gsMarginHor;
            int borderY = (2 * gsMarginVer) + mTextAdvance.height() + 4;
            uint8_t byteSize = mIsMetaData ? 1 : 2;
            int borderWidth = (levelWidth * (byteSize * (mTextAdvance.width() + mSpaceAdvance))) - mSpaceAdvance;
            int borderHeight = levelHeight * mTextAdvance.height();
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
    const uint8_t cutoff = mIsMetaData ? 8 : 16;

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
