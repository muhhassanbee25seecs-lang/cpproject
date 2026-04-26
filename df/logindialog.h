#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "forgotdialog.h" // Add this include

namespace Ui {
class logindialog;
}

class logindialog : public QDialog
{
    Q_OBJECT

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

private slots:
    void on_loginButton_clicked();
    void on_cancelButton_clicked();
    void on_forgotBtn_clicked();    // Added for Forgot Password logic
    void on_registerBtn_clicked();  // Optional: for the register link

private:
    Ui::logindialog *ui;
};

#endif 