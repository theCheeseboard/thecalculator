#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractListModel>

struct HistoryModelPrivate;
class HistoryModel : public QAbstractListModel {
        Q_OBJECT

    public:
        explicit HistoryModel(QObject* parent = nullptr);
        ~HistoryModel();

        struct HistoryItem {
                QString expression;
                QString result;
        };

        enum Roles {
            ExpressionRole,
            ResultRole
        };

        // Basic functionality:
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        void addHistoryEntry(HistoryItem entry);

    private:
        HistoryModelPrivate* d;

        // QAbstractItemModel interface
    public:
        QHash<int, QByteArray> roleNames() const override;
};

#endif // HISTORYMODEL_H
