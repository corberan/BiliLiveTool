#include "mainwindow.h"
#include <QApplication>

#include "update.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    Update update;
//    update.show();

    return a.exec();
}
