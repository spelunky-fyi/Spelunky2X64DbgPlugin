#pragma once

#include <QSize>
#include <QStyledItemDelegate>

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

namespace S2Plugin
{
    class StyledItemDelegateHTML : public QStyledItemDelegate
    {
        Q_OBJECT
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
