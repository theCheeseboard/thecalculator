#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("theCalculator");
    a.setOrganizationName("theSuite");

    QStringList args = a.arguments();
    args.removeFirst();

    MainWindow w;
    bool concatenateExpr;
    QString expr;
    for (QString arg : args) {
        if (concatenateExpr) {
            expr += " " + arg;
        } else {
            if (arg == "--help" || arg == "-h") {
                qDebug() << "theCalculator";
                qDebug() << "Usage: thecalculator [OPTIONS]";
                qDebug() << "  -e, --evaluate [EXPRESSION]  Evaluate the expression, print the result to standard output then exit";
                qDebug() << "                               THIS MUST BE THE LAST ARGUMENT. EVERYTHING AFTER THIS WILL BE TREATED AS THE EXPRESSION.";
                qDebug() << "                               Returns 0 if no error occurred, 2 if an error did occur.";
                qDebug() << "                               Read standard output for information about the error that occurred.";
                qDebug() << "      --degrees                Default to degrees. [DEFAULT]";
                qDebug() << "      --radians                Default to radians.";
                qDebug() << "  -h, --help                   Show this help output";
                qDebug() << "";
                qDebug() << "[EXPRESSION] is an expression to be calculated by theCalculator.";
                qDebug() << "             '*' indicates multiplication, '/' indicates division.";
                qDebug() << "             'pi' indicates the pi constant.";
                return 0;
            } else if (arg == "-e" || arg == "--evaluate") {
                concatenateExpr = true;
            } else if (arg == "--degrees") {
                w.setRadians(false);
            } else if (arg == "--radians") {
                w.setRadians(true);
            }
        }
    }

    if (concatenateExpr) {
        expr.remove(0, 1);

        bool error = false;
        expr.replace("pi", "π");
        expr.replace("+", " + ");
        expr.replace("-", " - ");
        expr.replace("×", " × ");
        expr.replace("÷", " ÷ ");
        expr.replace("*", " × ");
        expr.replace("/", " ÷ ");
        expr.replace("^", " ^ ");
        expr.replace("<<", " << ");
        expr.replace(">>", " >> ");
        QString answer = w.evaluate(expr, error, true);
        if (error) {
            qDebug("%s", QString("[ERROR] ").append(answer).toStdString().data());
            return 2;
        } else {
            qDebug("%s", answer.toStdString().data());
            return 0;
        }
    } else {
        w.show();
    }

    return a.exec();
}
