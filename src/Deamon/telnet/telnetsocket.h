#ifndef TELNETSOCKET_H
#define TELNETSOCKET_H

#include <QTcpSocket>
#include <libtelnet.h>

class TelnetServer;
class JSonModel;

class TelnetSocket : public QObject
{
    Q_OBJECT
public:
    explicit TelnetSocket(QTcpSocket *socket, TelnetServer* server);
    ~TelnetSocket();

    static void telnetEvent(telnet_t*, telnet_event_t* ev, void* user_data);

    void msgReveived(const QByteArray &msg);

    QString dispatchCmd(QString cmd);

    enum TelnetMode{
        Basic,
        Network,
        Patch,
        Application
    };

public slots:
    void sendMsg(QString str, bool release=true);
    void lockInput();
    void releaseInput();
signals:

protected slots:
    void readData();
    void socketDisconnected();

protected:
    QTcpSocket* _socket;
    TelnetServer* _server;

    telnet_t* _telnetInfo;

    TelnetMode _mode=Basic;
    JSonModel* model();
    void switchMode(TelnetMode m);

    bool _verbose=false;
    bool _locked=true;
    void setVerbose(bool verbose);

    QStringList splitArgs(QString args, bool comaSplit=false);
};

#endif // TELNETSOCKET_H
