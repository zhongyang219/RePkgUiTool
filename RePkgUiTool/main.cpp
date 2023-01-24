#include "RePkgUiTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RePkgUiTool w;
    w.show();
    return a.exec();
}
