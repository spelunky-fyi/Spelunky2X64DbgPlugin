#pragma once

#include <QBrush>
#include <QModelIndex>
#include <QPainter>
#include <QStyledItemDelegate>

namespace S2Plugin
{
    class StyledItemDelegateColorPicker : public QStyledItemDelegate
    {
        using QStyledItemDelegate::QStyledItemDelegate;

      protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
        {
            QStyleOptionViewItemV4 options = option;
            initStyleOption(&options, index);

            painter->save();

            auto adjusted = options.rect.adjusted(5, 5, -5, -5);
            auto color = index.data(Qt::DisplayRole).value<QColor>();
            painter->fillRect(adjusted, QBrush(color));
            painter->setPen(Qt::darkGray);
            painter->drawRect(adjusted);

            painter->restore();
        }
    };
} // namespace S2Plugin
