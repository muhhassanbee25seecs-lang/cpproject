QT += widgets
QT += core gui multimedia multimediawidgets
CONFIG += c++17
CONFIG += link_pkgconfig
PKGCONFIG += opencv4


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialog1.cpp \
    dialog2.cpp \
    dialog3.cpp \
    dialog4.cpp \
    main.cpp \
    mainwindow.cpp\
    dialog5.cpp\
    logindialog.cpp\
    forgotdialog.cpp\
    registerdialog.cpp

HEADERS += \
    dialog1.h \
    dialog2.h \
    dialog3.h \
    dialog4.h \
    mainwindow.h\
    dialog5.h\
    logindialog.h\
    forgotdialog.h\
    registerdialog.h

FORMS += \
    dialog1.ui \
    dialog2.ui \
    dialog3.ui \
    dialog4.ui \
    mainwindow.ui\
    dialog5.ui\
    logindialog.ui\
    forgotdialog.ui\
    registerdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    images/camera.png \
    images/report.png \
    images/upload.png

RESOURCES += \
    img.qrc \
    img2.qrc \
    img3.qrc \
    img5.qrc \
    img6.qrc
