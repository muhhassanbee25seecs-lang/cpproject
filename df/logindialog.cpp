#include "logindialog.h"
#include "ui_logindialog.h"
#include "forgotdialog.h" 
#include "registerdialog.h" 
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

logindialog::logindialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::logindialog)
{
    ui->setupUi(this);
}

logindialog::~logindialog()
{
    delete ui;
}

// Opens the Password Recovery Dialog
void logindialog::on_forgotBtn_clicked()
{
    ForgotDialog forgot(this);
    forgot.exec(); 
}

// Logic to check credentials in /app/inventory/credentials.txt
void logindialog::on_loginButton_clicked()
{
    // usernameLineEdit now represents the Professor's Name
    QString inputProfName = ui->usernameLineEdit->text().trimmed();
    QString inputPass = ui->passwordLineEdit->text();

    if(inputProfName.isEmpty() || inputPass.isEmpty()){
        QMessageBox::warning(this, "Login", "Please enter both Professor Name and Password");
        return;
    }

    // Path updated to the specific credentials file
    QFile file("/app/inventory/credentials.txt");
    bool loginSuccess = false;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            // Data format is Name:Password:SecurityAnswer
            QStringList parts = line.split(":");
            
            if (parts.size() >= 2) {
                // Check if Professor Name and Password match
                if (parts[0] == inputProfName && parts[1] == inputPass) {
                    loginSuccess = true;
                    break;
                }
            }
        }
        file.close();
    } else {
        // Fallback for first-time use if the file doesn't exist yet
        if(inputProfName == "admin" && inputPass == "123") {
            loginSuccess = true;
        } else {
            QMessageBox::critical(this, "System Error", "Credentials file not found. Please register first.");
            return;
        }
    }

    if (loginSuccess) {
        accept();
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid Professor Name or Password.");
    }
}

void logindialog::on_cancelButton_clicked()
{
    reject();
}

// Opens the Registration Dialog
void logindialog::on_registerBtn_clicked()
{
    registerdialog reg(this);
    reg.exec(); 
}