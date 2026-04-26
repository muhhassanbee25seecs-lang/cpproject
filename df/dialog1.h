#ifndef DIALOG1_H
#define DIALOG1_H

#include <QDialog>
#include <QImage>
#include <opencv2/opencv.hpp> // Required for cv::VideoCapture

namespace Ui { 
class Dialog1; 
}

class Dialog1 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog1(QWidget *parent = nullptr);
    ~Dialog1();

private slots:
    void on_btn_save_clicked();      // Saves attendance to /app/inventory
    void on_pushButton_clicked();    // Toggles ABSENT/PRESENT status
    void on_pushButton_2_clicked();  // Connects to IP Webcam and captures immediately
    void processConfigCapture();
private:
    Ui::Dialog1 *ui;
    
    // --- IP WEBCAM SYSTEM ---
    cv::VideoCapture m_cap;         
    // --- RECOGNITION PIPELINE ---
    QTimer *m_timer;
    void runRecognition();           // Executes ./recognize
    void readStudentInfo(QString folderName); // Reads data from /app/dataset/ID
    QString readConfigPath();
};

#endif