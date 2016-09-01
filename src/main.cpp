#include <QApplication>
#include "Chess.h"

int main (int argc, char *argv[])
{
    QApplication a(argc,argv);

    Chess c;
    c.show();

    return a.exec();
}
