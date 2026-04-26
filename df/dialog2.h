#ifndef DIALOG2_H
#define DIALOG2_H

#include <QDialog>

namespace Ui {
class Dialog2;
}

class Dialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog2(QWidget *parent = nullptr);
    ~Dialog2();
public:
    void addStudent(QString imagePath, QString name, QString id, QString roll);
private:
    Ui::Dialog2 *ui;
    void loadRecords();
};

#endif // DIALOG2_H
