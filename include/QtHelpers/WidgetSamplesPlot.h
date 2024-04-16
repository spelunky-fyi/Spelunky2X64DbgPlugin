#pragma once

#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

namespace S2Plugin
{
    class Logger;

    class WidgetSamplesPlot : public QWidget
    {
        Q_OBJECT
      public:
        WidgetSamplesPlot(Logger* logger, QWidget* parent = nullptr) : QWidget(parent), mLogger(logger)
        {
            setMouseTracking(true);
            setCursor(Qt::CrossCursor);
        }

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override
        {
            return minimumSizeHint();
        }

      protected:
        void paintEvent(QPaintEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override
        {
            mCurrentMousePos = event->pos();
            update();
        }
        void leaveEvent(QEvent* event) override
        {
            update();
        }

      private:
        Logger* mLogger;
        QPoint mCurrentMousePos = QPoint();
    };
} // namespace S2Plugin
