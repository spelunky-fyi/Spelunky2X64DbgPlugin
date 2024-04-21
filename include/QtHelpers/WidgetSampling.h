#pragma once

#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

namespace S2Plugin
{
    class WidgetSampling : public QWidget
    {
        Q_OBJECT
      public:
        using QWidget::QWidget;

        QSize minimumSizeHint() const override
        {
            return QSize(400, 400);
        }
        QSize sizeHint() const override
        {
            return minimumSizeHint();
        }

      protected:
        void paintEvent(QPaintEvent*) override
        {
            auto painter = QPainter(this);
            painter.save();

            painter.fillRect(rect(), Qt::white);
            painter.setPen(Qt::darkGray);
            painter.drawRect(rect().adjusted(0, 0, -1, -1));

            static const auto caption = QString("Sampling...");
            static const auto font = QFont("Arial", 16);
            static const auto captionSize = QFontMetrics(font).size(Qt::TextSingleLine, caption);

            painter.setFont(font);
            painter.setPen(Qt::lightGray);
            painter.drawText(QPointF((width() / 2.) - (captionSize.width() / 2.), (height() / 2.) - (captionSize.height() / 2.)), caption);

            painter.restore();
        }
    };
} // namespace S2Plugin
