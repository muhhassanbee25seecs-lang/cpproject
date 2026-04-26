#include "dialog4.h"
#include "ui_dialog4.h"
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QKeyEvent>
#include <QRegularExpression>
#include <opencv2/opencv.hpp>
#include <QInputDialog> // Required for VideoCapture

Dialog4::Dialog4(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog4) {
    ui->setupUi(this);
    ui->textEdit->installEventFilter(this);
    ui->textEdit_2->installEventFilter(this);

    timer = new QTimer(this);
    // Connect to the new OpenCV-based capture logic
    connect(timer, &QTimer::timeout, this, &Dialog4::processConfigCapture);
}

Dialog4::~Dialog4() {
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    delete ui;
}

QString Dialog4::readConfigPath() {
    QFile configFile("/app/camera.conf");
    QStringList options;

    // 1. Read all available IPs from the file
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

    // 2. Show a popup menu to choose the camera
    bool ok;
    QString selected = QInputDialog::getItem(this, "Select Camera", 
                                             "Choose the IP to use:", 
                                             options, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        return selected; // Return the specific IP you clicked
    }
    
    return QString(); // User cancelled
}

void Dialog4::processConfigCapture() {
    timer->stop();
    
    // This now triggers the selection popup
    QString url = readConfigPath();

    // If user cancelled or file was empty, reset button and exit
    if (url.isEmpty()) {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("CAPTURE PHOTO");
        return;
    }

    // 3. Attempt to open ONLY the selected IP
    if (!m_cap.open(url.toStdString())) {
        QMessageBox::warning(this, "Connection Error", "Cannot reach: " + url);
        ui->pushButton_2->setEnabled(true);
        ui->pushButton_2->setText("CAPTURE PHOTO");
        return;
    }

    // 4. Capture logic
    cv::Mat frame;
    for(int i = 0; i < 5; i++) {
        m_cap.read(frame);
    }

    if (!frame.empty()) {
        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
        QImage img((const uchar*) rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);
        processAndDisplayImage(img.copy());
        qDebug() << "Success: Image captured from " << url;
    } else {
        QMessageBox::critical(this, "Capture Error", "Received empty frame.");
    }

    m_cap.release(); 
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_2->setText("CAPTURE PHOTO");
}
void Dialog4::on_pushButton_2_clicked() {
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_2->setText("Connecting...");
    // 2-second delay to allow user to pose
    timer->start(2000); 
}


void Dialog4::processAndDisplayImage(const QImage &img) {
    // 1. Convert QImage to cv::Mat
    QImage swapped = img.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(swapped.height(), swapped.width(), CV_8UC3, (void*)swapped.bits(), swapped.bytesPerLine());

    // 2. Flip 90 degrees anticlockwise
    cv::Mat rotatedMat;
    cv::rotate(mat, rotatedMat, cv::ROTATE_90_COUNTERCLOCKWISE);

    // 3. Convert back to QImage
    QImage result(
        (const uchar*)rotatedMat.data, 
        rotatedMat.cols, 
        rotatedMat.rows, 
        rotatedMat.step, 
        QImage::Format_RGB888
    );

    capturedImage = result.copy();

    // 5. Display in the UI label
    ui->lbl_captured_image->setPixmap(QPixmap::fromImage(capturedImage).scaled(
        ui->lbl_captured_image->size(), 
        Qt::KeepAspectRatio, 
        Qt::SmoothTransformation
    ));

    qDebug();
}
void Dialog4::on_btnSave_clicked() {
    QString name = ui->textEdit->toPlainText().trimmed();
    QString roll = ui->textEdit_2->toPlainText().trimmed();

    QRegularExpression re("^[a-zA-Z0-9 ]*$");
    if (!re.match(roll).hasMatch() || capturedImage.isNull() || name.isEmpty() || roll.isEmpty()) {
        QMessageBox::warning(this, "Error", "Invalid inputs or no image captured.");
        return;
    }

    // Duplicate Check
    QDir baseDir("/app/dataset/");
    for (const QString &dirName : baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QFile file(baseDir.absoluteFilePath(dirName + "/info.txt"));
        if (file.open(QIODevice::ReadOnly)) {
            if (QTextStream(&file).readAll().contains("Roll: " + roll)) {
                QMessageBox::critical(this, "Duplicate", "Roll Number already registered!");
                return;
            }
        }
    }

    QString path = getNextFolderPath();
    // Save to the new student folder
    if (capturedImage.save(path + "/image.jpg", "JPG", 95)) {
        QFile info(path + "/info.txt");
        if (info.open(QIODevice::WriteOnly)) {
            QTextStream(&info) << "Name: " << name << "\nRoll: " << roll 
                               << "\nSaved At: " << QDateTime::currentDateTime().toString();
            info.close();
        }

        // --- TRAINING ---
        QProcess *trainProcess = new QProcess(this);
        trainProcess->setWorkingDirectory("/app/src");
        
        ui->btnSave->setEnabled(false);
        ui->btnSave->setText("Training...");

        connect(trainProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                [=](int exitCode, QProcess::ExitStatus exitStatus){
            
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                QMessageBox::information(this, "Success", "Student Registered and Model Updated!");
            } else {
                QMessageBox::warning(this, "Failed", "Training script failed.");
            }

            ui->btnSave->setEnabled(true);
            ui->btnSave->setText("SAVE TO DATASET");
            ui->textEdit->clear(); 
            ui->textEdit_2->clear();
            ui->lbl_captured_image->clear();
            ui->lbl_captured_image->setText("CAMERA PREVIEW");
            trainProcess->deleteLater();
        });

        trainProcess->start("./train", QStringList()); 
    }
}

QString Dialog4::getNextFolderPath() {
    QString basePath = "/app/dataset/";
    QDir dir(basePath);
    if (!dir.exists()) dir.mkpath(".");
    int index = 0;
    while (dir.exists(QString::number(index))) { index++; }
    dir.mkdir(QString::number(index));
    return basePath + QString::number(index);
}

bool Dialog4::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Space) return true;
    }
    return QDialog::eventFilter(obj, event);
}