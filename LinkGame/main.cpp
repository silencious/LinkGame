#include "linkgamegui.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LinkGameGUI w;
    w.show();

    return a.exec();
}
