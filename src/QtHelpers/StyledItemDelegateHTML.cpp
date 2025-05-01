#include "QtHelpers/StyledItemDelegateHTML.h"

#include <QAbstractTextDocumentLayout>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTextDocument>

void S2Plugin::StyledItemDelegateHTML::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    painter->save();

    QTextDocument doc;
    doc.setHtml(options.text);

    options.text = "";
    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);
    QSize iconSize = options.icon.actualSize(options.rect.size());
    if (mCenterVertically)
    {
        doc.setTextWidth(options.rect.width());
        auto centerVOffset = (options.rect.height() - doc.size().height()) / 2.0;
        painter->translate(options.rect.left(), options.rect.top() + centerVOffset);
    }
    else
    {
        painter->translate(options.rect.left() + iconSize.width(), options.rect.top() - 2);
    }
    QRect clip(0, 0, options.rect.width() + iconSize.width(), options.rect.height());
    // doc.drawContents(painter, clip);

    painter->setClipRect(clip);
    QAbstractTextDocumentLayout::PaintContext ctx;
    auto newColor = index.data(Qt::TextColorRole);
    if (!newColor.isNull())
        ctx.palette.setColor(QPalette::Text, newColor.value<QColor>());

    ctx.clip = clip;
    doc.documentLayout()->draw(painter, ctx);

    painter->restore();
}

QSize S2Plugin::StyledItemDelegateHTML::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(options.text);
    doc.setTextWidth(options.rect.width());
    doc.setDocumentMargin(2);
    return QSize(doc.idealWidth(), doc.size().height());
}
