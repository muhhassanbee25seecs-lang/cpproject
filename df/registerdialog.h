#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class registerdialog;
}

class registerdialog : public QDialog
{
    Q_OBJECT

public:
    explicit registerdialog(QWidget *parent = nullptr);
    ~registerdialog();

private slots:
    /**
     * @brief Handles the registration logic.
     * Validates input and saves Professor credentials to /app/inventory/credentials.txt
     */
    void on_registerBtn_clicked();

    /**
     * @brief Closes the registration dialog and returns to the login screen.
     */
    void on_backBtn_clicked();

private:
    Ui::registerdialog *ui;
};

#endif // REGISTERDIALOG_H