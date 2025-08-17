#pragma once

#include <QString>
#include <QWidget>
#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    struct ToolTipRect;

    class WidgetSpelunkyRooms : public QWidget
    {
        Q_OBJECT
      public:
        explicit WidgetSpelunkyRooms(const std::string& fieldName, QWidget* parent = nullptr);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

        void setOffset(size_t offset);
        void setIsMetaData()
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

      private:
        size_t mCurrentToolTip{0};
        QString mFieldName;
        size_t mOffset{0};
        QSize mTextAdvance;
        int mSpaceAdvance;
        bool mIsMetaData = false;
        std::vector<ToolTipRect> mToolTipRects;
        bool* mUseEnum{nullptr};
        bool* mShowPath{nullptr};
    };
} // namespace S2Plugin
