#include "telnetsocket.h"
#include "telnetserver.h"
#include "../model/networkmodel.h"

#include <QCoreApplication>
#include <QMetaMethod>
#include <functional>

void TelnetSocket::msgReveived(const QByteArray& msg)
{
    lockInput();
    QString cmd(msg);
    cmd.remove("\r\n");

    if(cmd=="#-1")
        switchMode(Basic);
    else if(cmd=="#0")
        switchMode(Network);
    else if(cmd=="#1")
        switchMode(Patch);
    else if(cmd=="#2")
        switchMode(Application);
    else if(cmd=="#v")
        setVerbose(true);
    else if(cmd=="#-v")
        setVerbose(false);
    else if(cmd=="#quit")
        qApp->quit();
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
        send("#Error: "+address+" not found in "+m->name());
        return;
    }

    cmd = cmd.mid(address.length()+(address.length()?1:0));

        // ---   FUNCTION HANDLING   ---
    if(cmd.contains('(')){
        QString function = cmd.left(cmd.indexOf('('));
        cmd = cmd.left(cmd.indexOf(')'));
        QStringList args = splitArgs(cmd.mid(cmd.indexOf('(')+1));
        std::function<void(QString)> cb = [this](QString r){
                    QMetaObject::invokeMethod(this, "send", Q_ARG(QString, r), Q_ARG(bool, true));
        };
        if(!n->call(function, args, cb))
            send("#Error: "+function + " doesn't exist.");
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
    case TelnetSocket::Patch:
        return 0;
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
    case TelnetSocket::Patch:
        send("#1: Patch");
        break;
    case TelnetSocket::Application:
        send("#2: Application");
        break;

    }

    if(_verbose)
        connect(model(), SIGNAL(out(QString)), this, SLOT(send(QString)));
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

    send(server->serverInfo());
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
            socket->msgReveived(QByteArray(ev->data.buffer, ev->data.size));
        return;
    case TELNET_EV_SEND:
        socket->_socket->write(ev->data.buffer, ev->data.size);
        return;
    default:
        return;
    }
}
void TelnetSocket::send(QString str, bool release)
{
    if(!_locked)
        telnet_printf(_telnetInfo, "\r");
    str = str+(str.isEmpty()?"":"\n");
    telnet_printf(_telnetInfo, str.toStdString().data());

    if(!_locked)
        telnet_printf(_telnetInfo, ">");
    else if(release)
        releaseInput();
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
    telnet_printf(_telnetInfo, ">");
    _locked = false;
    t.stop();
}
