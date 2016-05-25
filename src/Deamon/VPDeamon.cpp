#include <QCoreApplication>
#include "telnet/telnetserver.h"
#include "xbee/xbeeinterface.h"
#include "model/networkmodel.h"
#include "model/eventmodel.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    SXBeeInterface::init();
    SEventModel::init();
    SNetworkModel::init();
    STelnetServer::init();

    SXBeeInterface::ptr()->start();
    STelnetServer::ptr()->startServer(9000);

    int r= a.exec();
    SXBeeInterface::ptr()->terminate();
    SXBeeInterface::ptr()->wait();

    STelnetServer::clean();
    SNetworkModel::clean();
    SEventModel::clean();
    SXBeeInterface::clean();

    return r;
}
