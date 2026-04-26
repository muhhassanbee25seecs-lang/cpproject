#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>    // Added
#include <QTime>     // Added

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateTime(); // Added slot for the clock
    void on_start1_clicked();
    void on_removestudent_clicked();
    void on_startButton_clicked();
    void on_reportButton_clicked();
    void on_recordbtn_clicked();
    void on_riskbtn_clicked();
    void on_addstudent_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;    
};
#endif