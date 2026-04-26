#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

registerdialog::registerdialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::registerdialog) 
{
    ui->setupUi(this);
}

registerdialog::~registerdialog() { 
    delete ui; 
}

void registerdialog::on_registerBtn_clicked() {
    // These names now match your XML exactly
    QString profName = ui->idEdit->text().trimmed(); 
    QString pass = ui->passEdit->text();
    QString answer = ui->answerEdit->text().trimmed().toLower();

    // 1. Check if empty
    if(profName.isEmpty() || pass.isEmpty() || answer.isEmpty()) {
        QMessageBox::warning(this, "Registration", "Please fill all fields!");
        return;
    }

    // 2. Ensure directory exists
    QDir dir;
    if (!dir.exists("/app/inventory")) {
        dir.mkpath("/app/inventory");
    }

    // 3. Write to the credentials file
    QFile file("/app/inventory/credentials.txt");
    
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        // Format: Name:Password:Answer
        out << profName << ":" << pass << ":" << answer << "\n";
        file.close();
        
        QMessageBox::information(this, "Success", "Professor " + profName + " registered!");
        accept(); 
    } else {
        QMessageBox::critical(this, "File Error", "Cannot write to /app/inventory/credentials.txt. Check permissions.");
    }
}

void registerdialog::on_backBtn_clicked() {
    reject();
}