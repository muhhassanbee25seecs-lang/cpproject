#include "dialog2.h"
#include "ui_dialog2.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>

Dialog2::Dialog2(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog2)
{
    ui->setupUi(this);

    // 1. Setup Table Headers (Matching your XML with 5 columns)
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({"IMAGE", "NAME", "ID", "ROLL NO", "STATUS"});
    
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(85); 
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    loadRecords();
}

void Dialog2::loadRecords() {
    QString inventoryPath = "/app/inventory/attendance_records.txt";
    QString datasetPath = "/app/dataset/"; 
    
    QFile file(inventoryPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File not found!";
        return;
    }

    ui->tableWidget->setRowCount(0);
    QTextStream in(&file);

    // Added 'id' variable to collect the folder number
    QString name, roll, status, timestamp, id; 

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // 1. Detect the end of a block (the dashes)
        if (line.startsWith("---") || in.atEnd()) {
            if (!name.isEmpty() || !roll.isEmpty()) {
                int row = ui->tableWidget->rowCount();
                ui->tableWidget->insertRow(row);

                // --- Column 1: NAME ---
                ui->tableWidget->setItem(row, 1, new QTableWidgetItem(name));
                
                // --- Column 2: ID (Folder Name) ---
                // Now uses the actual ID parsed from the file
                ui->tableWidget->setItem(row, 2, new QTableWidgetItem(id)); 
                
                // --- Column 3: ROLL NO ---
                ui->tableWidget->setItem(row, 3, new QTableWidgetItem(roll));

                // --- Column 0: IMAGE ---
                // CRITICAL: We now use the parsed 'id' to find the folder
                QString imgPath = datasetPath + id + "/image.jpg"; 
                if (!id.isEmpty() && QFile::exists(imgPath)) {
                    QLabel *imgLabel = new QLabel();
                    QPixmap pix(imgPath);
                    imgLabel->setPixmap(pix.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    imgLabel->setAlignment(Qt::AlignCenter);
                    ui->tableWidget->setCellWidget(row, 0, imgLabel);
                } else {
                    ui->tableWidget->setItem(row, 0, new QTableWidgetItem("N/A"));
                }

                // --- Column 4: STATUS ---
                QLabel *statusLabel = new QLabel(status.isEmpty() ? "UNKNOWN" : status);
                statusLabel->setAlignment(Qt::AlignCenter);
                if (status.contains("PRESENT")) 
                    statusLabel->setStyleSheet("color: #2ecc71; font-weight: bold;");
                else 
                    statusLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
                
                ui->tableWidget->setCellWidget(row, 4, statusLabel);

                // Center align text items
                for(int col = 1; col < 4; ++col) {
                    if(ui->tableWidget->item(row, col))
                        ui->tableWidget->item(row, col)->setTextAlignment(Qt::AlignCenter);
                }

                // Clear variables for the next student block
                name.clear(); roll.clear(); status.clear(); timestamp.clear(); id.clear();
            }
            continue;
        }

        // 2. Extract data from lines inside the block
        if (line.startsWith("Name:"))           name = line.section(':', 1).trimmed();
        else if (line.startsWith("Roll:"))      roll = line.section(':', 1).trimmed();
        else if (line.startsWith("Status:"))    status = line.section(':', 1).trimmed();
        else if (line.startsWith("Timestamp:")) timestamp = line.section(':', 1).trimmed();
        else if (line.startsWith("ID:"))        id = line.section(':', 1).trimmed(); // <--- READ THE ID
    }
    file.close();
}
Dialog2::~Dialog2() {
    delete ui;
}