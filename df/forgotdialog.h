#ifndef FORGOTDIALOG_H
#define FORGOTDIALOG_H

#include <QDialog>

namespace Ui { class ForgotDialog; }

class ForgotDialog : public QDialog {
    Q_OBJECT
public:
    explicit ForgotDialog(QWidget *parent = nullptr);
    ~ForgotDialog();

private slots:
    void on_resetBtn_clicked();

private:
    Ui::ForgotDialog *ui;
};

#endif