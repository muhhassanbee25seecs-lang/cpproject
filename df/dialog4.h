#ifndef DIALOG4_H
#define DIALOG4_H

#include <QDialog>
#include <QTimer>
#include <QImage>
#include <QProcess>
#include <QNetworkAccessManager>
#include <opencv2/opencv.hpp> // 1. Add this include

namespace Ui { class Dialog4; }

class Dialog4 : public QDialog {
    Q_OBJECT

public:
    explicit Dialog4(QWidget *parent = nullptr);
    ~Dialog4();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_pushButton_2_clicked();
    void on_btnSave_clicked();
    void processConfigCapture();

private:
    Ui::Dialog4 *ui;
    QTimer *timer;
    QImage capturedImage;
    
    // 2. Declare the OpenCV VideoCapture object here
    cv::VideoCapture m_cap; 

    QString readConfigPath();
    void processAndDisplayImage(const QImage &img);
    QString getNextFolderPath();
};

#endif // DIALOG4_H