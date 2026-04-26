#include "mainwindow.h"
#include "logindialog.h" 
#include <QApplication>

int main(int argc, char *argv[])
{
    // High DPI scaling is often useful for modern displays
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QApplication a(argc, argv);

    logindialog login; 

    // The dialog blocks execution here until accept() or reject() is called
    if (login.exec() == QDialog::Accepted) {
        MainWindow w;
        w.show();
        return a.exec(); // Start the main event loop
    }

    return 0; // Exit if login was cancelled or failed
}