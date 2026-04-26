#include "dialog1.h"
#include "ui_dialog1.h"
#include <QDateTime>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <opencv2/opencv.hpp>

Dialog1::Dialog1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog1)
{
    ui->setupUi(this);
}

Dialog1::~Dialog1()
{
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    delete ui;
}

// Save Attendance Record
void Dialog1::on_btn_save_clicked()
{
    QString name = ui->lineEdit->text();
    QString roll = ui->lineEdit_roll->text();
    QString status = ui->lineEdit_2->text();
    QString id = ui->lineEdit->property("student_id").toString();
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    if (name.isEmpty() || name.contains("Unknown") || name.contains("Error")) {
        QMessageBox::warning(this, "Save Error", "No valid student recognized to save!");
        return;
    }

    QDir dir;
    if (!dir.exists("/app/inventory")) dir.mkpath("/app/inventory");

    QFile file("/app/inventory/attendance_records.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "------------------------------\n";
        out << "Timestamp: " << timestamp << "\n";
        out << "Name:      " << name << "\n";
        out << "Roll:      " << roll << "\n";
        out << "ID:        " << id << "\n";
        out << "Status:    " << status << "\n";
        out << "------------------------------\n";
        file.close();

        QMessageBox::information(this, "Success", "Attendance saved for " + name);
        
        // Reset UI
        ui->lineEdit->clear();
        ui->lineEdit_roll->clear();
        ui->lineEdit_2->setText("ABSENT");
        ui->lineEdit_2->setStyleSheet("color: red; background-color: #252a41; border-radius: 5px; font-weight: bold;");
    } else {
        QMessageBox::critical(this, "File Error", "Could not write to inventory file.");
    }
}

// 🔘 Toggle Present/Absent
void Dialog1::on_pushButton_clicked()
{
    if (ui->lineEdit->text().isEmpty() || ui->lineEdit->text().contains("Unknown")) return;

    if (ui->lineEdit_2->text() == "ABSENT") {
        ui->lineEdit_2->setText("PRESENT");
        ui->lineEdit_2->setStyleSheet("color: #2ecc71; background-color: #252a41; border: 1px solid #3a3a4f; border-radius: 5px; padding-left: 15px; font-size: 16px; font-weight: bold;");
    } else {
        ui->lineEdit_2->setText("ABSENT");
        ui->lineEdit_2->setStyleSheet("color: red; background-color: #252a41; border: 1px solid #3a3a4f; border-radius: 5px; padding-left: 15px; font-size: 16px; font-weight: bold;");
    }
}

// Capture from IP Webcam (Updated with 90° Anticlockwise Flip)
void Dialog1::on_pushButton_2_clicked()
{
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_2->setText("Capturing...");

    // 1. Read URL from camera.conf
    QFile file("/app/camera.conf");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "camera.conf missing in /app/");
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("START CAMERA");
        return;
    }
    QString url = QTextStream(&file).readLine().trimmed();
    file.close();

    // 2. Open Stream
    if (!m_cap.open(url.toStdString())) {
        QMessageBox::warning(this, "Connection Error", "Cannot reach IP Webcam.");
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("START CAMERA");
        return;
    }

    // 3. Flush buffer and grab frame
    cv::Mat frame;
    for(int i = 0; i < 5; i++) {
        m_cap.read(frame);
    }

    if (!frame.empty()) {
        // --- ROTATION LOGIC: 90 Degrees Anticlockwise ---
        cv::Mat rotatedFrame;
        cv::rotate(frame, rotatedFrame, cv::ROTATE_90_COUNTERCLOCKWISE);

        // Save for the recognize binary
        if(cv::imwrite("/app/test.jpg", rotatedFrame)) {
            runRecognition();
        } else {
            qDebug() << "ERROR: Could not save test.jpg";
        }
    }

    m_cap.release(); 
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_2->setText("START CAMERA");
}

// 🧠 Trigger Recognition Script
void Dialog1::runRecognition()
{
    QProcess *recognizeProcess = new QProcess(this);
    recognizeProcess->setWorkingDirectory("/app/src");

    connect(recognizeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, recognizeProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        
        QString fullOutput = recognizeProcess->readAllStandardOutput().trimmed();
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString studentId = "";
            QStringList lines = fullOutput.split("\n");

            for (const QString &line : lines) {
                if (line.contains("Recognized Student ID:", Qt::CaseInsensitive)) {
                    studentId = line.section(':', 1).section('|', 0).trimmed();
                    break; 
                }
            }

            if (!studentId.isEmpty() && studentId != "-1") {
                readStudentInfo(studentId); 
            } 
            else {
                ui->lineEdit->setText("Unknown");
                ui->lineEdit_roll->setText("N/A");
            }
        }
        recognizeProcess->deleteLater();
    });

    recognizeProcess->start("./recognize", QStringList());
}

// 📂 Read dataset/X/info.txt
void Dialog1::readStudentInfo(QString folderName)
{
    QString infoPath = "/app/dataset/" + folderName + "/info.txt";
    QFile file(infoPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->lineEdit->setProperty("student_id", folderName);

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("Name:", Qt::CaseInsensitive)) {
                ui->lineEdit->setText(line.section(':', 1).trimmed());
            }
            if (line.startsWith("Roll:", Qt::CaseInsensitive)) {
                ui->lineEdit_roll->setText(line.section(':', 1).trimmed());
            }
        }
        file.close();
        
        
    // Reset UI
    ui->lineEdit->clear();
    ui->lineEdit_roll->clear();
    ui->lineEdit_2->setText("ABSENT");
// Added border, padding, and font-size to match PRESENT style
    ui->lineEdit_2->setStyleSheet("color: red; background-color: #252a41; border: 1px solid #3a3a4f; border-radius: 5px; padding-left: 15px; font-size: 16px; font-weight: bold;");

    
    }
}