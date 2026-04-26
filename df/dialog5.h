#ifndef DIALOG5_H
#define DIALOG5_H

#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <QString> 
#include <opencv2/opencv.hpp>

namespace Ui { class Dialog5; }

class Dialog5 : public QDialog {
    Q_OBJECT

public:
    explicit Dialog5(QWidget *parent = nullptr);
    ~Dialog5();

private slots:
    void on_btnConfirmDelete_clicked();
    void on_btnCapturePhoto_clicked();

private:
    Ui::Dialog5 *ui;
    
    // --- IP Webcam Handling ---
    cv::VideoCapture m_cap;           // Handle for the IP stream
    QString readConfigPath();         // Reads URL from /app/camera.conf

    // --- Data Management ---
    void fetchStudentInfo(QString id); 
    void cleanLogs(QString id);

    // --- Image Processing ---
    QImage matToQImage(const cv::Mat &mat);
};

#endif 