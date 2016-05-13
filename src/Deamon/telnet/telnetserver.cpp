#include "telnetserver.h"
#include <QTcpSocket>
#include <QtCore>

TelnetServer::TelnetServer(QObject *parent) : QTcpServer(parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(newClient()));
    connect(this, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(connectError(QAbstractSocket::SocketError)));
}

bool TelnetServer::startServer(quint16 port)
{
    if(listen(QHostAddress::Any, port)){
        qWarning()<<"Server started on "<<port;
        return true;
    }
    qWarning()<<"Host fail:"<<errorString();
    return false;
}

void TelnetServer::stopServer()
{
    close();
}

void TelnetServer::newClient()
{
    while(hasPendingConnections()){
        TelnetSocket* s = new TelnetSocket(nextPendingConnection(), this);
        _sockets.append(s);
    }
}
void TelnetServer::deleteSocket(TelnetSocket* socket)
{
    if(_sockets.removeOne(socket))
        delete socket;
}


void TelnetServer::connectError(QAbstractSocket::SocketError )
{
    qWarning()<<"Telnet server error.";
}
