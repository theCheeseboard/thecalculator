#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QCommandLineParser>
#include <QTranslator>
#include <QLibraryInfo>

#include "calc.bison.hpp"

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
#endif

MainWindow* MainWin = nullptr;
QMap<QString, std::function<idouble(QList<idouble>,QString&)>> customFunctions;
QMap<QString, idouble> variables;
bool explicitEvaluation = false;
Result* res = nullptr;
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
        a = new QApplication(argc, argv);
    }

    a->setOrganizationName("theSuite");
    a->setOrganizationDomain("");
    a->setApplicationName("theShell");

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

    MainWindow::setupBuiltinFunctions();

    if (parser.value(expressionOption) == "") {
        res = new Result();
        MainWin = new MainWindow();
        MainWin->show();
        return a->exec();
    } else {
        explicitEvaluation = true;
        QList<QPair<QString, QString>> outputs;
        for (QString e : parser.value(expressionOption).split(":")) {
            e = e.remove(" "); //Remove all spaces

            if (res != nullptr) delete res;
            res = new Result();

            YY_BUFFER_STATE bufferState = yy_scan_string(QString(e + "\n").toUtf8().constData());
            yyparse();
            yy_delete_buffer(bufferState);

            if (resSuccess && !res->assigned) {
                outputs.append(QPair<QString, QString>(e, idbToString(res->result)));
            } else if (!resSuccess) {
                outputs.append(QPair<QString, QString>(e, res->error));
                break;
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
