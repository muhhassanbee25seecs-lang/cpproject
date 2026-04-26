#include "dialog5.h"
#include "ui_dialog5.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <opencv2/opencv.hpp>
#include <QPainter>
#include <QPainterPath>

Dialog5::Dialog5(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog5)
{
    ui->setupUi(this);
}

Dialog5::~Dialog5()
{
    if (m_cap.isOpened()) m_cap.release();
    delete ui;
}

//Helper: Read URL from camera.conf
QString Dialog5::readConfigPath() {
    QFile configFile("/app/camera.conf");
    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        QString line = in.readLine().trimmed();
        configFile.close();
        return line;
    }
    return QString();
}

//CAPTURE & RECOGNIZE
void Dialog5::on_btnCapturePhoto_clicked()
{
    ui->btnCapturePhoto->setEnabled(false);
    ui->btnCapturePhoto->setText("Connecting...");

    QString url = readConfigPath();
    if (url.isEmpty()) {
        QMessageBox::critical(this, "Error", "Config /app/camera.conf is missing or empty.");
        ui->btnCapturePhoto->setEnabled(true);
        ui->btnCapturePhoto->setText("CAPTURE PHOTO");
        return;
    }

    // Open IP Stream
    if (!m_cap.open(url.toStdString())) {
        QMessageBox::critical(this, "Connection Error", "Cannot reach IP Webcam: " + url);
        ui->btnCapturePhoto->setEnabled(true);
        ui->btnCapturePhoto->setText("CAPTURE PHOTO");
        return;
    }

    cv::Mat frame;
    // Flush buffer to get live frame
    for(int i = 0; i < 5; i++) { m_cap.read(frame); }

    if (frame.empty()) {
        QMessageBox::warning(this, "Capture Error", "Received empty frame.");
        m_cap.release();
        ui->btnCapturePhoto->setEnabled(true);
        return;
    }

    //ORIENTATION: 90 DEGREES ANTICLOCKWISE
    cv::Mat rotatedFrame;
    cv::rotate(frame, rotatedFrame, cv::ROTATE_90_COUNTERCLOCKWISE);

    // Save for recognition script
    cv::imwrite("/app/test.jpg", rotatedFrame);
    m_cap.release(); // Close stream

    // Show in UI
    cv::Mat rgbFrame;
    cv::cvtColor(rotatedFrame, rgbFrame, cv::COLOR_BGR2RGB);
    QImage qimg((const unsigned char*)(rgbFrame.data), rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
    
    //CONVERT TO CIRCLE MASK
    QPixmap originalPixmap = QPixmap::fromImage(qimg.copy());
    int size = qMin(ui->student_picture_frame->width(), ui->student_picture_frame->height());
    
    // Create a square transparent canvas
    QPixmap roundedPixmap(size, size);
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Create a circular clipping path
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);

    // Draw the original image onto the circle
    painter.drawPixmap(0, 0, size, size, originalPixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    painter.end();

    // Set the circular pixmap to the label
    ui->student_picture_frame->setPixmap(roundedPixmap);
    ui->student_picture_frame->setAlignment(Qt::AlignCenter);

    ui->btnCapturePhoto->setText("Recognizing...");

    // Start Recognition Process
    QProcess *recognizeProcess = new QProcess(this);
    recognizeProcess->setWorkingDirectory("/app/src"); 

    connect(recognizeProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, recognizeProcess](int exitCode, QProcess::ExitStatus exitStatus) {
        
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            QString fullOutput = recognizeProcess->readAllStandardOutput();
            QString studentId = "";

            QStringList lines = fullOutput.split("\n");
            for (const QString &line : lines) {
                if (line.contains("Recognized Student ID:", Qt::CaseInsensitive)) {
                    studentId = line.section(':', 1).section('|', 0).trimmed();
                    break;
                }
            }

            if (!studentId.isEmpty() && studentId != "-1") {
                ui->textEdit_id->setProperty("real_id", studentId);
                fetchStudentInfo(studentId);
            } else {
                ui->textEdit_id->setPlainText("Unknown Student");
                ui->textEdit_id->setProperty("real_id", "");
            }
        }
        ui->btnCapturePhoto->setEnabled(true);
        ui->btnCapturePhoto->setText("CAPTURE PHOTO");
        recognizeProcess->deleteLater();
    });

    recognizeProcess->start("./recognize", QStringList());
}

// 2. FETCH STUDENT NAME
void Dialog5::fetchStudentInfo(QString id) {
    QString infoPath = "/app/dataset/" + id + "/info.txt";
    QFile file(infoPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("Name:", Qt::CaseInsensitive)) {
                ui->textEdit_id->setPlainText(line.section(':', 1).trimmed());
                return;
            }
        }
    }
    ui->textEdit_id->setPlainText("ID: " + id);
}

// 3. CLEAN TEXT LOGS
void Dialog5::cleanLogs(QString id)
{
    QString filePath = "/app/inventory/attendance_records.txt";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return; 

    QStringList filteredContent;
    QStringList currentBlock;
    bool blockContainsId = false;
    QRegularExpression idPattern("^ID:\\s*" + QRegularExpression::escape(id.trimmed()) + "$");

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        currentBlock.append(line);
        if (line.trimmed().contains(idPattern)) blockContainsId = true;

        if (line.contains("------------------------------")) {
            if (!blockContainsId) filteredContent.append(currentBlock);
            currentBlock.clear();
            blockContainsId = false;
        }
    }
    if (!currentBlock.isEmpty() && !blockContainsId) filteredContent.append(currentBlock);
    file.close();

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&file);
        for (const QString &blockLine : filteredContent) out << blockLine << "\n";
        file.close();
    }
}

// 4. DELETE BUTTON
void Dialog5::on_btnConfirmDelete_clicked()
{
    QString studentId = ui->textEdit_id->property("real_id").toString();
    if (studentId.isEmpty()) studentId = ui->textEdit_id->toPlainText().trimmed();

    if (studentId.isEmpty() || studentId == "Unknown Student") {
        QMessageBox::warning(this, "Error", "Capture photo or enter ID to delete.");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirm Deletion", 
                                     "Delete logs and dataset for ID: " + studentId + "?",
                                     QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        cleanLogs(studentId);
        QDir dir("/app/dataset/" + studentId);
        bool folderRemoved = dir.exists() ? dir.removeRecursively() : false;

        if (folderRemoved) QMessageBox::information(this, "Success", "Data deleted.");
        else QMessageBox::information(this, "Logs Cleaned", "Logs removed, but folder not found.");

        ui->textEdit_id->clear();
        ui->student_picture_frame->clear();
        ui->textEdit_id->setProperty("real_id", "");
    }
}