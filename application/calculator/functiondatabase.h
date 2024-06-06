#ifndef FUNCTIONDATABASE_H
#define FUNCTIONDATABASE_H

#include <QObject>

struct FunctionDatabasePrivate;
class FunctionDatabase : public QObject {
        Q_OBJECT
    public:
        explicit FunctionDatabase(QObject* parent = nullptr);
        ~FunctionDatabase();

        struct Function {
                struct Overload {
                        struct Argument {
                                QString name;
                                QString description;
                        };

                        QString description;
                        QList<Argument> arguments;
                };

                QString name;
                QList<Overload> overloads;
        };

        bool haveFunction(QString name);
        Function function(QString name);

    signals:

    private:
        FunctionDatabasePrivate* d;
};

#endif // FUNCTIONDATABASE_H
