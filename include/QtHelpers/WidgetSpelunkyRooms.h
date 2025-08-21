#pragma once

#include "QtHelpers/WidgetMemoryView.h" // for ToolTipRect
#include <QString>
#include <QWidget>
#include <array>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class WidgetSpelunkyRooms : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetSpelunkyRooms(const std::string& fieldName, QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void setOffset(size_t offset);
        void setAsMetaData()
        {
            mIsMetaData = true;
        }
        void setNameSwitch(bool* b)
        {
            mUseEnum = b;
        }
        void setPathVisible(bool* b)
        {
            mShowPath = b;
        }

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        void leaveEvent(QEvent* ev) override
        {
            resetHover();
            QWidget::leaveEvent(ev);
        }

      public slots:
        void resetHover()
        {
            if (mHoverOverIndex == std::numeric_limits<uint8_t>::max())
                return;

            mHoverOverIndex = std::numeric_limits<uint8_t>::max();
            this->update();
        }

      private:
        static constexpr const size_t gsBufferSize = 8 * 15 * 2;     // 8x15 rooms * 2 bytes per room
        static constexpr const size_t gsHalfBufferSize = 8 * 15 * 1; // 8x15 rooms * 1 byte/bool per room

        size_t mCurrentToolTip{0};
        QString mFieldName;
        size_t mOffset{0};
        QSize mTextAdvance;
        int mSpaceAdvance;
        bool mIsMetaData = false;
        uint8_t mHoverOverIndex = std::numeric_limits<uint8_t>::max();
        std::array<ToolTipRect, gsHalfBufferSize> mRoomRects;
        bool* mUseEnum{nullptr};
        bool* mShowPath{nullptr};
    };
} // namespace S2Plugin
