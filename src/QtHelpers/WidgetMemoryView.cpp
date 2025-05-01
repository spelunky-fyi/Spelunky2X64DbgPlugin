#include "QtHelpers/WidgetMemoryView.h"

#include "pluginmain.h"
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QToolTip>
#include <cmath>

static constexpr int gsMarginHor = 10;
static constexpr int gsMarginVer = 5;

S2Plugin::WidgetMemoryView::WidgetMemoryView(QWidget* parent) : QWidget(parent)
{
    auto font = QFont("Courier", 11);
    mTextAdvance = QFontMetrics(font).size(Qt::TextSingleLine, "00");
    mSpaceAdvance = QFontMetrics(font).size(Qt::TextSingleLine, " ").width();
    setMouseTracking(true);
}

void S2Plugin::WidgetMemoryView::paintEvent(QPaintEvent*)
{
    if (mAddress != 0 && mSize != 0)
    {
        const static auto font = QFont("Courier", 11);
        QPainter painter(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        painter.setFont(font);

        // mToolTipRects.clear();
        painter.setBrush(Qt::black);
        int x = gsMarginHor;
        int y = gsMarginVer + mTextAdvance.height();
        size_t index = 0;
        bool addToolTips = mToolTipRects.empty();
        for (uintptr_t opCounter = mAddress; opCounter < (mAddress + mSize); ++opCounter)
        {
            // paint highlighted fields
            painter.setPen(Qt::transparent);
            for (const auto& field : mHighlightedFields)
            {
                if (opCounter == field.offset)
                {
                    auto rect = QRect(x, y - mTextAdvance.height() + 5, field.size * mTextAdvance.width() + ((field.size - 1) * mSpaceAdvance), mTextAdvance.height() - 2);
                    painter.setBrush(field.color);
                    painter.drawRoundedRect(rect, 4.0, 4.0);
                    if (addToolTips)
                        mToolTipRects.emplace_back(ToolTipRect{rect, QString::fromStdString(field.tooltip)});
                }
            }

            // paint hex values
            painter.setPen(QPen(Qt::SolidLine));
            auto str = QString("%1").arg(mMemoryData[opCounter - mAddress], 2, 16, QChar('0'));
            painter.drawText(x, y, str);
            x += mTextAdvance.width() + mSpaceAdvance;
            index++;
            if (index % 16 == 0)
            {
                y += mTextAdvance.height();
                x = gsMarginHor;
            }
        }
    }
}

QSize S2Plugin::WidgetMemoryView::sizeHint() const
{
    return minimumSizeHint();
}

QSize S2Plugin::WidgetMemoryView::minimumSizeHint() const
{
    int totalWidth = ((mTextAdvance.width() + mSpaceAdvance) * 16) + (gsMarginHor * 2) - mSpaceAdvance;
    int totalHeight = (mTextAdvance.height() * static_cast<int>(std::ceil(static_cast<double>(mSize) / 16.))) + (gsMarginVer * 2) + mTextAdvance.height();
    return QSize(totalWidth, totalHeight);
}

void S2Plugin::WidgetMemoryView::setOffsetAndSize(size_t offset, size_t size)
{
    mAddress = offset;
    mSize = size;
    update();
    updateGeometry();
    adjustSize();
}

void S2Plugin::WidgetMemoryView::clearHighlights()
{
    mHighlightedFields.clear();
    mToolTipRects.clear();
    update();
}

void S2Plugin::WidgetMemoryView::addHighlightedField(std::string tooltip, size_t offset, int size, QColor color)
{
    mHighlightedFields.emplace_back(std::move(tooltip), offset, size, std::move(color));
}

void S2Plugin::WidgetMemoryView::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = event->pos();
    for (const auto& ttr : mToolTipRects)
    {
        if (ttr.rect.contains(pos))
        {
            QToolTip::showText(mapToGlobal(pos), ttr.tooltip);
            return;
        }
    }
}

void S2Plugin::WidgetMemoryView::updateMemory()
{
    Script::Memory::Read(mAddress, &mMemoryData, gBigEntityBucket, nullptr);
    update();
}
