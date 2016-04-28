#include "telnetsocket.h"

#include "telnetserver.h"


void TelnetSocket::msgReveived(const QByteArray& msg)
{

}

/************************************************************
 *                      SOCKET HANDLING                     *
 ************************************************************/

TelnetSocket::TelnetSocket(QTcpSocket *socket, TelnetServer *server)
    : QObject(server)
{
    const telnet_telopt_t telnetOption[] = { {-1,0,0} };
    _telnetInfo = telnet_init( telnetOption, telnetEvent, 0, this);
    _socket = socket;
    _server = server;

    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    sendMsg(server->serverInfo());
}

TelnetSocket::~TelnetSocket()
{
    delete _socket;
    telnet_free(_telnetInfo);
}

void TelnetSocket::readData()
{
    QByteArray data = _socket->readAll();
    if(data.isEmpty())
            return;

    telnet_recv(_telnetInfo, data.data(), data.size());
}

void TelnetSocket::socketDisconnected()
{
    _server->deleteSocket(this);
}
/************************************************************
 *                      TELNET HANDLING                     *
 ************************************************************/

void TelnetSocket::telnetEvent(telnet_t*, telnet_event_t *ev, void *user_data)
{
    TelnetSocket* socket = static_cast<TelnetSocket*>(user_data);

    switch(ev->type){
    case TELNET_EV_DATA:
        socket->msgReveived(QByteArray(ev->data.buffer, ev->data.size));
        return;
    case TELNET_EV_SEND:
        socket->_socket->write(ev->data.buffer, ev->data.size);
        return;
    default:
        return;
    }
}
void TelnetSocket::sendMsg(QString str)
{
    telnet_printf(_telnetInfo, str.toStdString().data());
}
