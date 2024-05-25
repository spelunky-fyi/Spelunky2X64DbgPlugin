#pragma once

#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

namespace S2Plugin
{
    class StyledItemDelegateHTML : public QStyledItemDelegate
    {
      public:
        using QStyledItemDelegate::QStyledItemDelegate;
        void setCenterVertically(bool b)
        {
            mCenterVertically = b;
        }

      protected:
        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

      private:
        bool mCenterVertically = false;
    };
} // namespace S2Plugin
