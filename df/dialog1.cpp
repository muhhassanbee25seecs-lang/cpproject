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
#include <QInputDialog> // Added for the selection popup

Dialog1::Dialog1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog1)
{
    ui->setupUi(this);
    // Initialize timer for the capture delay
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Dialog1::processConfigCapture);
}

Dialog1::~Dialog1()
{
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    delete ui;
}

// 📂 Logic to read all IPs and show a Selection Popup
QString Dialog1::readConfigPath() {
    QFile configFile("/app/camera.conf");
    QStringList options;

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty()) {
                options << line;
            }
        }
        configFile.close();
    }

    if (options.isEmpty()) return QString();

    bool ok;
    QString selected = QInputDialog::getItem(this, "Select Camera", 
                                             "Choose the IP to use:", 
                                             options, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        return selected;
    }
    
    return QString();
}

// 📸 Capture Logic triggered after the timer
void Dialog1::processConfigCapture() {
    m_timer->stop();
    
    QString url = readConfigPath();

    if (url.isEmpty()) {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("START CAMERA");
        return;
    }

    if (!m_cap.open(url.toStdString())) {
        QMessageBox::warning(this, "Connection Error", "Cannot reach: " + url);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("START CAMERA");
        return;
    }

    cv::Mat frame;
    for(int i = 0; i < 5; i++) {
        m_cap.read(frame);
    }

    if (!frame.empty()) {
        // --- ROTATION LOGIC: 90 Degrees Anticlockwise ---
        cv::Mat rotatedFrame;
        cv::rotate(frame, rotatedFrame, cv::ROTATE_90_COUNTERCLOCKWISE);

        if(cv::imwrite("/app/test.jpg", rotatedFrame)) {
            runRecognition();
        } else {
            qDebug() << "ERROR: Could not save test.jpg";
        }
    } else {
        QMessageBox::critical(this, "Capture Error", "Received empty frame.");
    }

    m_cap.release(); 
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_2->setText("START CAMERA");
}

// Start button clicked
void Dialog1::on_pushButton_2_clicked()
{
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_2->setText("Connecting...");
    m_timer->start(2000); 
}

// 🧠 Trigger Recognition Script (Unchanged)
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

// 📂 Read dataset/X/info.txt (Unchanged)
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
        
        ui->lineEdit_2->setText("ABSENT");
        ui->lineEdit_2->setStyleSheet("color: red; background-color: #252a41; border: 1px solid #3a3a4f; border-radius: 5px; padding-left: 15px; font-size: 16px; font-weight: bold;");
    }
}

// 💾 Save Attendance Record (Unchanged)
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
        
        ui->lineEdit->clear();
        ui->lineEdit_roll->clear();
        ui->lineEdit_2->setText("ABSENT");
        ui->lineEdit_2->setStyleSheet("color: red; background-color: #252a41; border: 1px solid #3a3a4f; border-radius: 5px; padding-left: 15px; font-size: 16px; font-weight: bold;");
    } else {
        QMessageBox::critical(this, "File Error", "Could not write to inventory file.");
    }
}

// 🔘 Toggle Present/Absent (Unchanged)
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