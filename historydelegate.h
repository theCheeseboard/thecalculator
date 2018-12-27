#ifndef HISTORYDELEGATE_H
#define HISTORYDELEGATE_H

#include <QObject>
#include <QAbstractItemDelegate>
#include <QPainter>

class HistoryDelegate : public QAbstractItemDelegate
{
        Q_OBJECT
    public:
        explicit HistoryDelegate(QObject *parent = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    signals:

    public slots:
};

#endif // HISTORYDELEGATE_H
