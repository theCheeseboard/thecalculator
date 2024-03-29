/****************************************
 *
 *   theCalculator - Calculator
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "evaluation/baseevaluationengine.h"
#include "mainwindow.h"
#include <QCommandLineParser>
#include <QFile>
#include <QLibraryInfo>
#include <QTranslator>
#include <tapplication.h>

#include "graph/graphfunction.h"
#include "graph/graphview.h"
#include <QPainter>
#include <libthecalculator_global.h>
#include <tstylemanager.h>

#ifdef Q_OS_MAC
    #include <CoreFoundation/CFBundle.h>
#endif

MainWindow* MainWin = nullptr;

int main(int argc, char* argv[]) {
    // Determine whether to start a QApplication or QCoreApplication
    for (int i = 1; i < argc; i++) {
        if (qstrcmp(argv[i], "-e") == 0 || qstrcmp(argv[i], "--evaluate") == 0 || qstrcmp(argv[i], "-g") == 0 || qstrcmp(argv[i], "--graph") == 0) {
            qputenv("QT_QPA_PLATFORM", "offscreen"); // Start offscreen so we don't create a connection to the X server
        }
    }

    tApplication a(argc, argv);
    a.setApplicationShareDir("thecalculator");
    a.addLibraryTranslator(LIBTHECALCULATOR_TRANSLATOR);

    a.installTranslators();

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("");
    a.setApplicationVersion("3.0");
    a.setGenericName(QApplication::translate("main", "Calculator"));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2023");
    //    a.setApplicationUrl(tApplication::HelpContents, QUrl("https://help.vicr123.com/docs/thecalculator/intro"));
    a.setApplicationUrl(tApplication::Sources, QUrl("http://github.com/vicr123/theCalculator"));
    a.setApplicationUrl(tApplication::FileBug, QUrl("http://github.com/vicr123/theCalculator/issues"));
    a.setApplicationName(T_APPMETA_READABLE_NAME);
    a.setDesktopFileName(T_APPMETA_DESKTOP_ID);

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::translate("main", "Calculator"));
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();
    parser.addOptions({
        {{"g", "graph"}, QApplication::translate("main", "Generate a graph in PNG format and write the data to stdout.")},
        {{"e", "evaluate"}, QApplication::translate("main", "Evaluate <expression>, print the result to standard output, then exit."), QApplication::translate("main", "expression")},
        {{"t", "trig-unit"}, QApplication::translate("main", "Use <unit> as the trigonometry unit. Possible values are degrees, radians and gradians."), QApplication::translate("main", "unit")},
        {{"c", "nocolor"}, QApplication::translate("main", "Do not output colour.")}
    });
    parser.parse(a.arguments());

    if (parser.isSet(versionOption)) parser.showVersion(); // Show version and kill the app here

    QMap<QString, QString> termCol;
    if (!parser.isSet("c")) {
        termCol = {
            {"reset",  "\033[0m" },
            {"bold",   "\033[1m" },
            {"red",    "\033[31m"},
            {"yellow", "\033[33m"}
        };
    }

    if (parser.isSet("g")) {
        QCommandLineParser parser;
        parser.setApplicationDescription(QApplication::translate("main", "Calculator"));
        parser.clearPositionalArguments();
        parser.addHelpOption();
        parser.addOptions({
            {{"g", "graph"}, QApplication::translate("main", "Generate a graph in PNG format and write the data to stdout.")},
            {"cx", QApplication::translate("main", "X value to center the generated graph at"), QApplication::translate("main", "x-value")},
            {"cy", QApplication::translate("main", "Y value to center the generated graph at"), QApplication::translate("main", "y-value")},
            {"sx", QApplication::translate("main", "Number of pixels to put between each integer in the X direction"), QApplication::translate("main", "x-scale")},
            {"sy", QApplication::translate("main", "Number of pixels to put between each integer in the Y direction"), QApplication::translate("main", "y-scale")},
            {{"o", "outfile"}, QApplication::translate("main", "File to output the graph to. If missing, output to stdout"), QApplication::translate("main", "path")}
        });
        parser.addPositionalArgument("width", QApplication::translate("main", "Width of the graph, in pixels"), "-g width");
        parser.addPositionalArgument("height", QApplication::translate("main", "Height of the graph, in pixels"));
        parser.addPositionalArgument("expressions", QApplication::translate("main", "Expressions to graph"), "expressions...");
        parser.process(a);

        // Ensure all the command line options are valid
        QTextStream out(stdout);
        QTextStream err(stderr);
        if (parser.positionalArguments().count() < 3) {
            err << "thecalculator: " + QApplication::translate("main", "missing operand") + "\n";
            err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
            err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
            return 1;
        }

        QStringList args = parser.positionalArguments();
        bool widthOk, heightOk;
        int width = args.takeFirst().toInt(&widthOk);
        int height = args.takeFirst().toInt(&heightOk);

        if (!widthOk || width < 1) {
            err << "thecalculator: " + QApplication::translate("main", "invalid output width") + "\n";
            err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
            err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
            return 1;
        }

        if (!heightOk || height < 1) {
            err << "thecalculator: " + QApplication::translate("main", "invalid output height") + "\n";
            err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
            err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
            return 1;
        }

        QImage image(width, height, QImage::Format_ARGB32);
        QPainter p(&image);
        GraphView graph;

        QPointF centerPoint(0, 0);
        if (parser.isSet("cx")) {
            bool cxOk;
            centerPoint.setX(parser.value("cx").toDouble(&cxOk));
            if (!cxOk) {
                err << "thecalculator: " + QApplication::translate("main", "invalid center x position") + "\n";
                err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
                err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
                return 1;
            }
        }

        if (parser.isSet("cy")) {
            bool cyOk;
            centerPoint.setY(parser.value("cy").toDouble(&cyOk));
            if (!cyOk) {
                err << "thecalculator: " + QApplication::translate("main", "invalid center y position") + "\n";
                err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
                err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
                return 1;
            }
        }

        graph.setCenter(centerPoint);

        if (parser.isSet("sx")) {
            bool sxOk;
            graph.setXScale(parser.value("sx").toDouble(&sxOk));
            if (!sxOk) {
                err << "thecalculator: " + QApplication::translate("main", "invalid x scale value") + "\n";
                err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
                err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
                return 1;
            }
        }

        if (parser.isSet("sy")) {
            bool syOk;
            graph.setYScale(parser.value("sy").toDouble(&syOk));
            if (!syOk) {
                err << "thecalculator: " + QApplication::translate("main", "invalid y scale value") + "\n";
                err << QApplication::translate("main", "Usage: %1 [options] -g width height expressions...").arg(a.arguments().first()) + "\n";
                err << "       " + QApplication::translate("main", "%1 -gh for more information.").arg(a.arguments().first()) + "\n";
                return 1;
            }
        }

        for (const QString& expr : qAsConst(args)) {
            GraphFunction* f = new GraphFunction(&graph, expr);
            graph.addFunction(f);
        }
        graph.render(&p, image.size());

        QFile outputFile;
        if (parser.isSet("o")) {
            outputFile.setFileName(parser.value("o"));
            outputFile.open(QFile::WriteOnly);
        } else {
            outputFile.open(stdout, QFile::WriteOnly);
        }

        if (!outputFile.isOpen()) {
            err << "thecalculator: " + QApplication::translate("main", "unable to open output file for writing") + "\n";
            return 1;
        }

        image.save(&outputFile, "PNG");

        return 0;
    } else {
        if (parser.isSet(helpOption)) parser.showHelp(); // Show help and kill the app here

        if (parser.value("e") == "") {
            MainWin = new MainWindow();
            MainWin->show();
            return a.exec();
        } else {
            auto engine = BaseEvaluationEngine::current();
            QVector<QPair<QString, QString>> outputs;
            QMap<QString, idouble> variables;
            bool didError = false;

            if (parser.isSet("t")) {
                QString unit = parser.value("t");
                if (unit == "degrees") {
                    BaseEvaluationEngine::current()->setTrigonometricUnit(BaseEvaluationEngine::Degrees);
                } else if (unit == "radians") {
                    BaseEvaluationEngine::current()->setTrigonometricUnit(BaseEvaluationEngine::Radians);
                } else if (unit == "gradians") {
                    BaseEvaluationEngine::current()->setTrigonometricUnit(BaseEvaluationEngine::Gradians);
                } else {
                    QTextStream err(stderr);
                    err << "thecalculator: " + QApplication::translate("main", "invalid trigonometric unit") + "\n";
                    err << "               " + QApplication::translate("main", "Available units are: degrees|radians|gradians") + "\n";
                    err << "               " + QApplication::translate("main", "%1 -h for more information.").arg(a.arguments().first()) + "\n";
                    return 2;
                }
            }

            for (QString e : parser.value("e").split(":")) {
                e = e.remove(" "); // Remove all spaces

                BaseEvaluationEngine::Result res = QCoro::waitFor(engine->evaluate(e, variables));

                switch (res.type) {
                    case BaseEvaluationEngine::Result::Scalar:
                        outputs.append(QPair<QString, QString>(e, idbToString(res.result)));
                        break;
                    case BaseEvaluationEngine::Result::Equality:
                        outputs.append(QPair<QString, QString>(e, res.isTrue ? QApplication::translate("MainWindow", "TRUE") : QApplication::translate("MainWindow", "FALSE")));
                        break;
                    case BaseEvaluationEngine::Result::Assign:
                        variables.insert(res.identifier, res.value);
                        break;
                    case BaseEvaluationEngine::Result::Error:
                        {
                            QStringList errorText;
                            errorText.append(termCol.value("yellow") + res.error + termCol.value("reset"));

                            QString locationText = QApplication::translate("MainWindow", "Location") + ": ";

                            QString errorExpression;
                            errorExpression = locationText;
                            errorExpression.append(e.left(res.location));
                            errorExpression.append(termCol.value("red"));
                            errorExpression.append(e.mid(res.location, res.length));
                            errorExpression.append(termCol.value("reset"));
                            errorExpression.append(e.mid(res.location + res.length));
                            errorText.append(errorExpression);

                            if (termCol.count() == 0 || res.length == 0) {
                                QString location;
                                QString here = QApplication::translate("MainWindow", "Here").toUpper();

                                // Have at least one arrow
                                if (res.length == 0) res.length = 1;

                                if (res.location + locationText.length() < here.length() + 1) {
                                    location.fill(' ', res.location + location.length());
                                    location.append(QString().fill('^', res.length));
                                    location.append(" ");
                                    location.append(here);
                                } else {
                                    location.fill(' ', locationText.length() + res.location - here.length() - 1);
                                    location.append(here);
                                    location.append(" ");
                                    location.append(QString().fill('^', res.length));
                                }
                                errorText.append(location);
                            }

                            outputs.append(QPair<QString, QString>(e, errorText.join("\n")));
                            break;
                        }
                }

                if (res.type == BaseEvaluationEngine::Result::Error) {
                    didError = true;
                    break; // Abort calculations here
                }
            }

            QTextStream out(stdout);
            if (outputs.count() == 0) {
                out << QApplication::translate("main", "Nothing to evaluate").append("\n");
            } else if (outputs.count() == 1) {
                out << outputs.first().second.append("\n");
            } else {
                for (const QPair<QString, QString>& output : outputs) {
                    out << termCol.value("bold") << output.first << ":" << termCol.value("reset") << "\n"
                        << output.second << "\n";
                }
            }

            if (!didError) {
                return 0;
            } else {
                return 1;
            }
        }
    }
}
