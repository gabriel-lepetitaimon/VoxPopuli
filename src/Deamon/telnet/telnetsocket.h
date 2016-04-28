#ifndef TELNETSOCKET_H
#define TELNETSOCKET_H

#include <QTcpSocket>
#include <libtelnet.h>

class TelnetServer;

class TelnetSocket : public QObject
{
    Q_OBJECT
public:
    explicit TelnetSocket(QTcpSocket *socket, TelnetServer* server);
    ~TelnetSocket();

    static void telnetEvent(telnet_t*, telnet_event_t* ev, void* user_data);

    void msgReveived(const QByteArray &msg);

public slots:
    void sendMsg(QString str);

signals:

protected slots:
    void readData();
    void socketDisconnected();

protected:
    QTcpSocket* _socket;
    TelnetServer* _server;

    telnet_t* _telnetInfo;

};

#endif // TELNETSOCKET_H
