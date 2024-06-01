#include "historymodel.h"

struct HistoryModelPrivate {
        QList<HistoryModel::HistoryItem> historyItems;
};

HistoryModel::HistoryModel(QObject* parent) :
    QAbstractListModel(parent), d{new HistoryModelPrivate()} {
}

HistoryModel::~HistoryModel() {
    delete d;
}

int HistoryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;

    return d->historyItems.count();
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};

    auto item = d->historyItems.at(index.row());
    switch (role) {
        case ExpressionRole:
            return item.expression;
        case ResultRole:
            return item.result;
    }

    return {};
}

void HistoryModel::addHistoryEntry(HistoryItem entry) {
    beginInsertRows({}, d->historyItems.count(), d->historyItems.count());
    d->historyItems.append(entry);
    endInsertRows();
}

QHash<int, QByteArray> HistoryModel::roleNames() const {
    return {
        {ExpressionRole, "expression"},
        {ResultRole,     "result"    }
    };
}
