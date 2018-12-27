#include "historydelegate.h"
#include "the-libs_global.h"

HistoryDelegate::HistoryDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{

}

void HistoryDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString text = index.data(Qt::DisplayRole).toString();
    QString firstText = text.split("=").first().trimmed();
    QString secondText = "= " + text.split("=").at(1).trimmed();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::transparent);
        painter->setBrush(option.palette.color(QPalette::Highlight));
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    } else if (option.state & QStyle::State_MouseOver) {
        QColor col = option.palette.color(QPalette::Highlight);
        col.setAlpha(127);
        painter->setBrush(col);
        painter->setPen(Qt::transparent);
        painter->drawRect(option.rect);
        painter->setBrush(Qt::transparent);
        painter->setPen(option.palette.color(QPalette::WindowText));
    } else {
        painter->setPen(option.palette.color(QPalette::WindowText));
    }

    QRect textRect;
    textRect.setWidth(option.rect.width() - 18);
    textRect.setLeft(option.rect.left() + 9);
    textRect.setTop(option.rect.top() + 9);
    textRect.setHeight(option.fontMetrics.height());

    painter->setFont(option.font);
    painter->drawText(textRect, firstText);

    QFont f = option.font;
    f.setBold(true);
    f.setPointSize(f.pointSize() + 5);

    textRect.moveTop(textRect.bottom());
    textRect.setHeight(QFontMetrics(f).height());

    painter->setFont(f);
    painter->drawText(textRect, Qt::AlignRight, secondText);

}

QSize HistoryDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size;
    size.setWidth(0);

    int height = option.fontMetrics.height();

    QFont f = option.font;
    f.setPointSize(f.pointSize() + 5);

    height += QFontMetrics(f).height();
    size.setHeight(height + 18);
    return size;
}
