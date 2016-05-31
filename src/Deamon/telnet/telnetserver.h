#ifndef TELNETSERVER_H
#define TELNETSERVER_H

#include <QObject>
#include <QTcpServer>

#include "telnetsocket.h"
#include "singleton.h"

typedef singleton<TelnetServer> STelnetServer;

class TelnetServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TelnetServer(QObject *parent = 0);

    void deleteSocket(TelnetSocket* socket);
    QString serverInfo(){return "VPDeamon v1.0\n";}

    void init(){}

signals:

public slots:
    bool startServer(quint16 port);
    void stopServer();

protected slots:
    void newClient();
    void connectError(QAbstractSocket::SocketError);

protected:
    QList<TelnetSocket*> _sockets;
};

#endif // TELNETSERVER_H
