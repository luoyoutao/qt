#include "sheettest.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SheetTest w;
    w.show();
    return a.exec();
}

