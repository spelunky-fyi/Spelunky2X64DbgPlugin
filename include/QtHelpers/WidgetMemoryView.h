#pragma once

#include "Data/Entity.h" // for gBigEntityBucket
#include <QColor>
#include <QRect>
#include <QString>
#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    struct HighlightedField
    {
        std::string tooltip;
        size_t offset;
        int size;
        QColor color;
        HighlightedField(std::string _tooltip, size_t _offset, int _size, QColor _color) : tooltip(_tooltip), offset(_offset), size(_size), color(_color){};
    };

    struct ToolTipRect
    {
        QRect rect;
        QString tooltip;
    };

    class WidgetMemoryView : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetMemoryView(QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void setOffsetAndSize(size_t offset, size_t size);

        void clearHighlights();
        void addHighlightedField(std::string tooltip, size_t offset, int size, QColor color);

        void updateMemory();

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;

      private:
        uintptr_t mAddress{0};
        size_t mSize{0};
        QSize mTextAdvance;
        int mSpaceAdvance;

        std::vector<HighlightedField> mHighlightedFields;
        std::vector<ToolTipRect> mToolTipRects;
        uint8_t mMemoryData[gBigEntityBucket] = {};
    };
} // namespace S2Plugin
