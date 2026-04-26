#include "forgotdialog.h"
#include "ui_forgotdialog.h"
#include <QMessageBox>
#include <QInputDialog>

ForgotDialog::ForgotDialog(QWidget *parent) : 
    QDialog(parent), 
    ui(new Ui::ForgotDialog) 
{
    ui->setupUi(this);
}

ForgotDialog::~ForgotDialog() { 
    delete ui; 
}

void ForgotDialog::on_resetBtn_clicked() {
    QString id = ui->idEdit->text();
    QString answer = ui->answerEdit->text().toLower().trimmed();

    
    if(id == "admin" && answer == "hassan") {
        
        bool ok;
        QString newPass = QInputDialog::getText(this, "Reset Password",
                                              "Identity Verified!\nEnter New Password:", 
                                              QLineEdit::Password, "", &ok);
        if (ok && !newPass.isEmpty()) {
            QMessageBox::information(this, "Success", "Password updated successfully!");
            accept();
        }
    } else {
        QMessageBox::critical(this, "Access Denied", "Incorrect ID or Security Answer.");
    }
}