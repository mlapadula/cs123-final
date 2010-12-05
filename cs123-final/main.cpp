#include <QtGui/QApplication>
#include <iostream>
#include <qgl.h>
#include <pty.h>
#include "mainwindow.h"
using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    cout << "cs123 final project" << endl;
    MainWindow w;
    w.show();
    return a.exec();
}
