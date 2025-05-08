#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowIcon(QIcon(R"(://winicon.ico)"));
    w.setWindowTitle(R"(TTML TRANS TOOL)");
    return a.exec();
}
