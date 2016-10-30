#include "telnetsocket.h"
#include "telnetserver.h"
#include "../model/networkmodel.h"
#include "../model/eventmodel.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <functional>

void TelnetSocket::cmdReceived(const QString& cmd)
{
    lockInput();

    if(cmd=="#-1")
        switchMode(Basic);
    else if(cmd=="#0")
        switchMode(Network);
    else if(cmd=="#1")
        switchMode(EventsProcessor);
    else if(cmd=="#2")
        switchMode(Application);
    else if(cmd=="#v")
        setVerbose(true);
    else if(cmd=="#-v")
        setVerbose(false);
    else if(cmd=="#friendly")
        setFriendlyCLI(true);
    else if(cmd=="#-friendly")
        setFriendlyCLI(false);
    else if(cmd=="#quit")
        qApp->quit();
    else if(cmd=="#q")
        _socket->disconnectFromHost();
    else
        dispatchCmd(cmd);
}

void TelnetSocket::dispatchCmd(QString cmd)
{
    JSonModel* m=model();

    if(!m)
        return;

    QString address = "";
    if(cmd.contains('.'))
        address = cmd.left(cmd.lastIndexOf('.'));
    JSonNode* n = m->nodeByAddress(address);
    if(!n){
        send("#Error: "+address+" not found in "+m->name(), true);
        return;
    }

    cmd = cmd.mid(address.length()+(address.length()?1:0));

        // ---   FUNCTION HANDLING   ---
    if(cmd.contains('(')){
        QString function = cmd.left(cmd.indexOf('('));
        cmd = cmd.left(cmd.indexOf(')'));
        QStringList args = splitArgs(cmd.mid(cmd.indexOf('(')+1), true);
        std::function<void(QString)> cb = [this](QString r){
                    QMetaObject::invokeMethod(this, "send", Q_ARG(QString, r), Q_ARG(bool, true));
        };
        if(!n->call(function, args, cb))
            send("#Error: "+function + " doesn't exist.", true);
        return;

        // ---  SET VARIABLE HANDLING  ---
    }else if(cmd.contains(':')){
        QString property = cmd.left(cmd.indexOf(':'));
        QString value = cmd.mid(property.length()+1);
        while(value.size()>0&&value.at(0)==' ')
            value.remove(0,1);

        JSonNode::SetError e = n->set(property, value);
        switch(e){
        case JSonNode::SameValue:
        case JSonNode::NoError:
            releaseInput();
            return;
        case JSonNode::DoesNotExist:
            send("#Error: "+property+" is not a member of "+address);
            return;
        case JSonNode::ReadOnly:
            send( "#Error: "+property+" is read only");
            return;
        case JSonNode::WrongArg:
            send("#Error: wrong value");
            return;
        }

        // ---  GET VARIABLE HANDLING  ---
    }else if(cmd.split(' ',QString::SkipEmptyParts).size()==1){
        while(cmd.at(cmd.size()-1)==' ')
            cmd.remove(cmd.size()-1);
        QString data = "";
        if(!n->getToString(cmd,data)){
            send("#Error: "+address+(address.isEmpty()?"":".")+cmd+" doesn't exist.");
            return;
        }
        send(address+(address.isEmpty()?"":".")+cmd+": "+data);
        return;
    }

    releaseInput();
}

JSonModel *TelnetSocket::model()
{
    switch(_mode){
    case TelnetSocket::Basic:
       return 0;
    case TelnetSocket::Network:
        return SNetworkModel::ptr();
    case TelnetSocket::EventsProcessor:
        return SEventModel::ptr();
    case TelnetSocket::Application:
        return 0;
    }

    return 0;
}

void TelnetSocket::switchMode(TelnetSocket::TelnetMode m)
{
    if(_verbose)
        disconnect(model(), SIGNAL(out(QString)), this, SLOT(send(QString)));

    _mode = m;

    switch(m){
    case TelnetSocket::Basic:
        send("#-1: Basic");
        break;
    case TelnetSocket::Network:
        send("#0: Network");
        break;
    case TelnetSocket::EventsProcessor:
        send("#1: Event Processor");
        break;
    case TelnetSocket::Application:
        send("#2: Application");
        break;
    }
    releaseInput();

    if(_verbose)
        connect(model(), SIGNAL(out(QString)), this, SLOT(send(QString)));
}

void TelnetSocket::setFriendlyCLI(bool friendly)
{
    if(friendly==_friendly)
        return;

    telnet_negotiate(_telnetInfo, friendly?TELNET_WILL:TELNET_WONT, TELNET_TELOPT_ECHO);
    telnet_negotiate(_telnetInfo, TELNET_DO, TELNET_TELOPT_LINEMODE);
    if(friendly){
        char linemode[] = {1,0};//EDIT:1 TRAPSIG:2 MODE_ACK:4 SOFT_TAB:8 LIT_ECHO:16
        telnet_subnegotiation(_telnetInfo, TELNET_TELOPT_LINEMODE, linemode, 2);
    }else{
        char linemode[] = {1,1+2};//EDIT:1 TRAPSIG:2 MODE_ACK:4 SOFT_TAB:8 LIT_ECHO:16
        telnet_subnegotiation(_telnetInfo, TELNET_TELOPT_LINEMODE, linemode, 2);
    }

    _friendly = friendly;
    releaseInput();
}

void TelnetSocket::setVerbose(bool verbose)
{
    if(verbose == _verbose)
        return;
    _verbose = verbose;
    if(verbose){
        connect(model(), SIGNAL(out(QString)), this, SLOT(send(QString)));
        send("#v: Verbose ON");
    }else{
        disconnect(model(), SIGNAL(out(QString)), this, SLOT(send(QString)));
        send("#-v: Verbose OFF");
    }
    releaseInput();
}


/************************************************************
 *                      GENERIC FUNCTION                    *
 ************************************************************/
QStringList TelnetSocket::splitArgs(QString args, bool comaSplit)
{
    QStringList r;

    while(args.size()>0 && (args.at(0)==' '||args.at(0)=='\t'))
        args.remove(0,1);
    while(args.size()>0 && (args.at(args.size()-1)==' '||args.at(args.size()-1)=='\t'))
        args.remove(args.size()-1,1);

    if(args.isEmpty())
        return r;

    QString c;
    bool quoted=false;

    for(int i=0; i<args.size(); i++){
        if(args.at(i)=='\\'){
            i++;
            if(i<args.size())
                c+=args.at(i);
            continue;
        }

        if(args.at(i)=='"'){
            if(quoted){
                quoted = false;
                r.append(c);
                c.clear();
            }else{
                quoted = true;
            }
            continue;
        }else if(!quoted && (args.at(i)==' ' || args.at(i)=='\t' || (comaSplit && args.at(i)==','))){
            if(!c.isEmpty()){
                r.append(c);
                c.clear();
            }
            continue;
        }
        c+=args.at(i);
    }

    if(!c.isEmpty())
        r.append(c);

    return r;
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
    socket->setParent(this);
    _server = server;

    connect(socket, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    _friendly = true;   //trigger the not friendly setup
    setFriendlyCLI(false);
    send(server->serverInfo(), true);
}

TelnetSocket::~TelnetSocket()
{
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
        if(!socket->_locked)
            socket->processInput(QByteArray(ev->data.buffer, ev->data.size));
        return;
    case TELNET_EV_SEND:
        socket->_socket->write(ev->data.buffer, ev->data.size);
        return;
    case TELNET_EV_ERROR:
        qWarning()<<"Telnet error: "<<QString(ev->error.msg);
        return;
    default:
        return;
    }
}

bool TelnetSocket::processInput(const QByteArray &input){
    if(!input.size())
        return false;

    QString msg = QString(input);
    if(!_friendly){
        cmdReceived(msg.remove("\r\n"));
        return true;
    }

    if(input=="\x1B[D"){        //right
        moveCursor(false);
    }else if(input=="\x1B[C"){  //left
        moveCursor(true);
    }else if(input=="\x1B[A"){   //up
        if(_histPos <= 0)
            return true;
        eraseLine();
        cliWrite(_cmdHistory.at(--_histPos));
    }else if(input=="\x1B[B"){   //down
        if(_histPos >= _cmdHistory.size())
            return true;
        eraseLine();
        _histPos++;
        if(_histPos<_cmdHistory.length())
            cliWrite(_cmdHistory.at(_histPos));
    }else if( msg.startsWith("\r")){ //enter
        _cmdHistory.append(_currentLine);
        _histPos = _cmdHistory.length();

        telnetWrite("\n");

        cmdReceived(_currentLine);

        _linePos = 0;
        _currentLine = "";
    }else if(input == "\x7F"){      //backspace
        if(_linePos<=0)
            return false;
        _currentLine.remove(--_linePos,1);
        updateCLI();
    }else if(msg[0]!='\x1' && msg[0]!='\n' && msg[0]!='\0' && msg[0]!='\t' && msg[0]!='\b'){
        cliWrite(msg);
    }


    return true;
}

void TelnetSocket::cliWrite(QString output)
{
    if(_linePos == _currentLine.length()){
        _currentLine.append(output);
        _linePos += output.length();
        telnetWrite(output);
    }else{
        _currentLine.insert(_linePos, output);
        telnetWrite(_currentLine.mid(_linePos));
        _linePos += output.length();
        for(int i=_currentLine.length(); i>_linePos;i--) moveTelnetCursor(false);
    }
}

bool TelnetSocket::moveCursor(bool toLeft)
{
    if(toLeft){     // To the left
        if(_linePos>=_currentLine.size())
            return false;
        _linePos++;
    }else{          // To the right
        if(_linePos<=0)
            return false;
        _linePos--;
    }
    moveTelnetCursor(toLeft);
    return true;
}

void TelnetSocket::moveTelnetCursor(bool toLeft)
{
    if(toLeft)
        telnetWrite("\x1B[C");
    else
        telnetWrite("\x1B[D");
}

void TelnetSocket::eraseLine(bool resetInternal)
{
    QString output = "\r";
    for(int i=0; i<_currentLine.length(); i++)       output += " ";
    output += "  \r>";
    telnetWrite(output);

    if(resetInternal){
        _linePos = 0;
        _currentLine = "";
    }
}

void TelnetSocket::updateCLI()
{
    eraseLine(false);
    telnetWrite(_currentLine);
    for(int i=_currentLine.length(); i>_linePos;i--) moveTelnetCursor(false);
}

void TelnetSocket::send(QString str, bool release)
{
    if(str.isEmpty())
        return;

    if(_friendly){
        QString erase = "\r";
        for(int i=0; i<_currentLine.length(); i++)       erase += " ";
        erase += "\r";
        telnetWrite(erase);
    }

    str = str+"\n";
    telnetWrite(str);

    if(!_locked){
        if(_friendly){
            telnetWrite(">");
            updateCLI();
        }
    }else if(release)
        releaseInput();
}

void TelnetSocket::telnetWrite(QString data){
    telnet_printf(_telnetInfo, data.toStdString().data());
}

void TelnetSocket::lockInput(){
    if(_locked)
        return;
    _locked = true;
    t.singleShot(2000, this, SLOT(releaseInput()));
}

void TelnetSocket::releaseInput()
{
    if(!_locked)
        return;
    if(_friendly) telnet_printf(_telnetInfo, ">");
    _locked = false;
    t.stop();
}
