#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "thread1.h"
#include "thread2.h"
#include "pingthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);
    bool isShowMainWin();

    void setRegEdit(const QString& master, const QString& gateway, const QString& shut);


public slots:
    void slot_setConfigIP();

private:
    Ui::MainWindow *ui;
    bool          showMainWin;
    Timer1Handler h1;
    PingHandler   hgateway;
    PingHandler   hself;
    //Timer1Handler h1second;
    Timer2Handler h2;
};

#endif // MAINWINDOW_H
