#include "threadsManager.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    threadsManager w;
    w.show();
    return a.exec();
}
