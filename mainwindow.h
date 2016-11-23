#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QtMath>
#include <QAction>
#include <QPropertyAnimation>
#include <QTableWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QParallelAnimationGroup>
#include <QLayout>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString evaluate(QString expression, bool &error, bool suppressOutput = false);
    void setRadians(bool radians);
private slots:

    void on_lineEdit_returnPressed();

    void on_evaluateButton_clicked();

    void on_lineEdit_cursorPositionChanged(int arg1, int arg2);

    void on_clearButton_clicked();

    void on_oneButton_clicked();

    void on_twoButton_clicked();

    void on_threeButton_clicked();

    void on_fourButton_clicked();

    void on_fiveButton_clicked();

    void on_sixButton_clicked();

    void on_sevenButton_clicked();

    void on_eightButton_clicked();

    void on_nineButton_clicked();

    void on_zeroButton_clicked();

    void on_pointButton_clicked();

    void on_plusButton_clicked();

    void on_minusButton_clicked();

    void on_multiplyButton_clicked();

    void on_divideButton_clicked();

    void on_openBracketButton_clicked();

    void on_closeBracketButton_clicked();

    void on_piButton_clicked();

    void on_backspaceButton_clicked();

    void on_sinButton_clicked();

    void on_cosButton_clicked();

    void on_tanButton_clicked();

    void on_actionRadians_triggered();

    void on_actionDegrees_triggered();

    void on_actionExit_triggered();

    void on_powerButton_clicked();

    void on_exponentButton_clicked();

    void backspace();

    void on_reciporicalButton_clicked();

    void on_AnsButton_clicked();

    void on_logButton_clicked();

    void on_lnButton_clicked();

    void on_xButton_clicked();

    void on_yButton_clicked();

    void on_functionsButton_clicked();

    void on_functionsClose_clicked();

    void on_functionsNew_clicked();

    void on_doneButton_clicked();

    void on_functionsInsert_clicked();

    void on_functionsTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_functionsDelete_clicked();

    void on_functionsEdit_clicked();

    void on_shiftButton_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    bool eventFilter(QObject *watched, QEvent *event);
    bool suppressCursorChecking;

    QSettings settings;

    int editingFunction = -1;

    QString previousAnswer = "0";
};

#endif // MAINWINDOW_H
