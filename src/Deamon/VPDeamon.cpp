#include <QCoreApplication>
#include "telnet/telnetserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    TelnetServer s;
    s.startServer(9000);

    return a.exec();
}
