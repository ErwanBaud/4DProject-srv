#include <QApplication>
#include "Serveur.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Serveur srv;
    srv.show();

    return app.exec();
}
