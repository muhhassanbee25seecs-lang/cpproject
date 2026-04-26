#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog1.h"
#include "dialog2.h"
#include "dialog3.h"
#include "dialog4.h"
#include "dialog5.h"
#include <QTimer>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Styling
    this->setStyleSheet(R"(
        QMainWindow { background-color: #0f172a; }
        QWidget { background-color: #0f172a; color: white; }
    )");

    
    // Make the time display look like a clean digital clock
    ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->timeEdit->setReadOnly(true);
    
    // Set format to "8:20 AM" style
    ui->timeEdit->setDisplayFormat("h:mm AP");

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); // Trigger every 1 second (1000ms)

    updateTime(); // Show time immediately on startup
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    // QTime::currentTime() fetches the local system time (Pakistan GMT+5)
    ui->timeEdit->setTime(QTime::currentTime());
}

// --- NAVIGATION SLOTS ---

void MainWindow::on_start1_clicked()
{
    Dialog1 dlg(this);
    dlg.exec();
}

void MainWindow::on_startButton_clicked()
{
    Dialog1 dlg(this);
    dlg.exec();
}

void MainWindow::on_removestudent_clicked() 
{
    Dialog5 d5(this);
    d5.exec();
}

void MainWindow::on_reportButton_clicked()
{
    Dialog2 *dlg = new Dialog2(this);
    dlg->show();
}

void MainWindow::on_recordbtn_clicked()
{
    Dialog2 dlg(this);
    dlg.exec();
}

void MainWindow::on_riskbtn_clicked()
{
    Dialog3 dlg(this);
    dlg.exec();
}

void MainWindow::on_addstudent_clicked()
{
    Dialog4 dlg(this);
    dlg.exec();
}