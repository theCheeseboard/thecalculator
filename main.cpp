#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>

MainWindow* MainWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineOption expressionOption(QStringList() << "e" << "evaluate");
    expressionOption.setDescription(QApplication::translate("main", "Evaluate <expression>, print the result to standard output, then exit"));
    expressionOption.setValueName(QApplication::translate("main", "expression"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Calculator");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(expressionOption);
    parser.process(a);

    MainWin = new MainWindow();
    if (parser.value(expressionOption) == "") {
        MainWin->show();
        return a.exec();

    } else {
        QList<QPair<QString, QString>> outputs;
        for (QString e : parser.value(expressionOption).split(":")) {
            QString result = MainWin->evaluateExpression(e) + "\n";
            if (!result.contains(QApplication::translate("main", "assigned to"))) {
                outputs.append(QPair<QString, QString>(e, result.trimmed()));
            }
        }

        QTextStream out(stdout);
        if (outputs.count() == 0) {
            out << QApplication::translate("main", "Nothing to evaluate").append("\n");
        } else if (outputs.count() == 1) {
            out << outputs.first().second.append("\n");
        } else {
            for (QPair<QString, QString> output : outputs) {
                out << output.first.append(": ").append(output.second).append("\n");
            }
        }
    }
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
