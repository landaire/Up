#include <QtWidgets/QApplication>
#include "MainForm.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainForm w;
    w.show();

    return a.exec();
}
