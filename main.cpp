#include "mainwindow.h"
#include <QApplication>

MainWindow* MainWin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWin = new MainWindow();
    MainWin->show();

    return a.exec();
}
