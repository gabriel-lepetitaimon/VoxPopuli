#ifndef TELNETSOCKET_H
#define TELNETSOCKET_H

#include <QTcpSocket>
#include <QTimer>
#include <libtelnet.h>
#include <string>

class TelnetServer;
class JSonModel;

class TelnetSocket : public QObject
{
    Q_OBJECT
public:
    explicit TelnetSocket(QTcpSocket *socket, TelnetServer* server);
    ~TelnetSocket();

    static void telnetEvent(telnet_t*, telnet_event_t* ev, void* user_data);

    void cmdReceived(const QString &cmd);

    void dispatchCmd(QString cmd);

    enum TelnetMode{
        Basic,
        Network,
        EventsProcessor,
        Application
    };

public slots:
    void send(QString str, bool release=false);
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

    bool _friendly;
    void setFriendlyCLI(bool friendly);

    bool _verbose=false;
    bool _locked=true;
    void setVerbose(bool verbose);

    QStringList splitArgs(QString args, bool comaSplit=false);
    inline void telnetWrite(QString data);

    // -- Friendly CLI handler --
    bool processInput(const QByteArray &input);
    bool moveCursor(bool toLeft);
    bool moveCursor(int deltaPos);
    void moveTelnetCursor(bool toLeft);
    void cliWrite(QString output);
    void eraseLine(bool resetInternal=true);
    void updateCLI();
    QString autoComplete(QString uncompleteLine, int &cursorPos, bool showPossibilites = false);
    QStringList _cmdHistory;
    QString _currentLine;
    int _linePos=0, _histPos;

private:
    QTimer t, autoCompleteTimer;
};

#endif // TELNETSOCKET_H
