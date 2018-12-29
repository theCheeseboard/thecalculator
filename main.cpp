#include "mainwindow.h"
#include <tapplication.h>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QTranslator>
#include <QLibraryInfo>
#include "evaluationengine.h"

#include "calc.bison.hpp"

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
#endif

MainWindow* MainWin = nullptr;
QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
bool explicitEvaluation = false;
bool resSuccess = false;
bool isDegrees = true;

QString numberFormatToString(long double number) {
    std::stringstream stream;
    stream << std::setprecision(10) << number;

    return QString::fromStdString(stream.str());
}

QString idbToString(idouble db) {
    long double real = db.real();
    long double imag = db.imag();
    if (real != 0 && imag == 0) {
        return numberFormatToString(real);
    } else if (real == 0 && imag == 0) {
        return "0";
    } else if (real != 0 && imag == 1) {
        return numberFormatToString(real) + " + i";
    } else if (real != 0 && imag > 0) {
        return numberFormatToString(real) + " + " + numberFormatToString(imag) + "i";
    } else if (real != 0 && imag == -1) {
        return numberFormatToString(imag) + " - i";
    } else if (real != 0 && imag < 0) {
        return numberFormatToString(real) + " - " + numberFormatToString(-imag) + "i";
    } else if (imag == 1) {
        return "i";
    } else if (imag == -1) {
        return "-i";
    } else {
        return numberFormatToString(imag) + "i";
    }
}

int main(int argc, char *argv[])
{
    //Determine whether to start a QApplication or QCoreApplication
    QCoreApplication* a = nullptr;
    for (int i = 1; i < argc; i++) {
        if (qstrcmp(argv[i], "-e") == 0 || qstrcmp(argv[i], "--evaluate") == 0) {
            a = new QCoreApplication(argc, argv);
        }
    }

    if (a == nullptr) {
        a = new tApplication(argc, argv);
    }

    a->setOrganizationName("theSuite");
    a->setOrganizationDomain("");
    a->setApplicationName("theCalculator");

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a->installTranslator(&qtTranslator);

    QTranslator localTranslator;
#ifdef Q_OS_MAC
    a.setAttribute(Qt::AA_DontShowIconsInMenus, true);

    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());

    localTranslator.load(QLocale::system().name(), QString::fromLocal8Bit(pathPtr) + "/Contents/translations/");

    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif

#ifdef Q_OS_LINUX
    localTranslator.load(QLocale::system().name(), "/usr/share/thecalculator/translations");
#endif

    a->installTranslator(&localTranslator);


    QCommandLineOption expressionOption(QStringList() << "e" << "evaluate");
    expressionOption.setDescription(QApplication::translate("main", "Evaluate <expression>, print the result to standard output, then exit"));
    expressionOption.setValueName(QApplication::translate("main", "expression"));

    QCommandLineParser parser;
    parser.setApplicationDescription("Calculator");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(expressionOption);
    parser.process(*a);

    MainWindow::setupFunctions();

    if (parser.value(expressionOption) == "") {
        MainWin = new MainWindow();
        MainWin->show();
        return a->exec();
    } else {
        EvaluationEngine engine;
        QList<QPair<QString, QString>> outputs;
        QMap<QString, idouble> variables;
        for (QString e : parser.value(expressionOption).split(":")) {
            e = e.remove(" "); //Remove all spaces

            engine.setExpression(e);
            engine.setVariables(variables);
            EvaluationEngine::Result res = engine.evaluate();

            switch (res.type) {
                case EvaluationEngine::Result::Scalar:
                    outputs.append(QPair<QString, QString>(e, idbToString(res.result)));
                    break;
                case EvaluationEngine::Result::Equality:
                    outputs.append(QPair<QString, QString>(e, res.isTrue ? QApplication::translate("MainWindow", "TRUE") : QApplication::translate("MainWindow", "FALSE")));
                    break;
                case EvaluationEngine::Result::Assign:
                    variables.insert(res.identifier, res.value);
                    break;
                case EvaluationEngine::Result::Error:
                    outputs.append(QPair<QString, QString>(e, res.error));
                    break;
            }

            if (res.type == EvaluationEngine::Result::Error) break; //Abort calculations here
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

        if (resSuccess) {
            return 0;
        } else {
            return 1;
        }
    }
}

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}
