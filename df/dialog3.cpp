#include "dialog3.h"
#include "ui_dialog3.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMap>
#include <QDebug>

// Struct to track attendance math
struct StudentStats {
    int total = 0;
    int present = 0;
    QString roll;
};

Dialog3::Dialog3(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog3)
{
    ui->setupUi(this);

    // 1. Table Formatting
    ui->risktable->setColumnCount(5);
    ui->risktable->setHorizontalHeaderLabels({"STUDENT NAME", "TOTAL CLASSES", "PRESENT", "PERCENTAGE", "STATUS"});
    ui->risktable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->risktable->verticalHeader()->setDefaultSectionSize(60);
    ui->risktable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    loadRiskData();
}

void Dialog3::loadRiskData() {
    QString path = "/app/inventory/attendance_records.txt";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open inventory file!";
        return;
    }

    // Map to group entries by Name/Roll
    QMap<QString, StudentStats> aggregator;
    QTextStream in(&file);

    QString currentName, currentStatus;

    // --- 1. PARSE AND AGGREGATE ---
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("Name:")) {
            currentName = line.section(':', 1).trimmed();
        } else if (line.startsWith("Status:")) {
            currentStatus = line.section(':', 1).trimmed();
            
            // Once we have a name and status, update the count
            if (!currentName.isEmpty()) {
                aggregator[currentName].total++;
                if (currentStatus.toUpper() == "PRESENT") {
                    aggregator[currentName].present++;
                }
            }
        }
    }
    file.close();

    // --- 2. DISPLAY CALCULATIONS ---
    ui->risktable->setRowCount(0);
    QMapIterator<QString, StudentStats> i(aggregator);
    
    while (i.hasNext()) {
        i.next();
        QString name = i.key();
        int total = i.value().total;
        int present = i.value().present;
        
        // Calculate Percentage
        double pct = (total > 0) ? ((double)present / total) * 100.0 : 0.0;
        QString pctStr = QString::number(pct, 'f', 1) + "%";

        int row = ui->risktable->rowCount();
        ui->risktable->insertRow(row);

        // Populate items
        ui->risktable->setItem(row, 0, new QTableWidgetItem(name));
        ui->risktable->setItem(row, 1, new QTableWidgetItem(QString::number(total)));
        ui->risktable->setItem(row, 2, new QTableWidgetItem(QString::number(present)));
        ui->risktable->setItem(row, 3, new QTableWidgetItem(pctStr));

        // --- 3. RISK STATUS LOGIC ---
        QLabel *statusLabel = new QLabel();
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setMargin(10);

        if (pct < 50.0) {
            statusLabel->setText("CRITICAL RISK");
            statusLabel->setStyleSheet("color: #ff5f57; font-weight: bold; background: rgba(255,95,87,0.1); border-radius: 5px;");
        } else if (pct < 75.0) {
            statusLabel->setText("WARNING");
            statusLabel->setStyleSheet("color: #ffbd2e; font-weight: bold; background: rgba(255,189,46,0.1); border-radius: 5px;");
        } else {
            statusLabel->setText("SAFE");
            statusLabel->setStyleSheet("color: #27c93f; font-weight: bold; background: rgba(39,201,63,0.1); border-radius: 5px;");
        }
        ui->risktable->setCellWidget(row, 4, statusLabel);

        // Center align text columns
        for(int col=0; col<4; col++) {
            if(ui->risktable->item(row, col))
                ui->risktable->item(row, col)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

Dialog3::~Dialog3() {
    delete ui;
}