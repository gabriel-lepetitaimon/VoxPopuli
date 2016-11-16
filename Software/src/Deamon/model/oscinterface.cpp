#include "oscinterface.h"

/********************************************
 *             OSC Interface                *
 *******************************************/
OscInterface::OscInterface(EventModel * eventModel)
    :JSonNode("OSC", eventModel)
{
    _oscListener = new qosc::Listener(this);
    _oscListener->addListener(this);
}
OscInterface::~OscInterface()
{
    delete _oscListener;
}

void OscInterface::setOscAddress(const QString& address)
{
    _oscAddress = address;
    if(!_oscAddress.startsWith('/')) _oscAddress.prepend('/');
    if(!_oscAddress.endsWith('/'))   _oscAddress.append('/');
    
    _oscListener->setAddressStartFilter(_oscAddress);
}

void OscInterface::setPort(int port)
{
    _oscListener->startServer(port);
}

void OscInterface::readOscMsg(QString addr, QVariantList args)
{
    qWarning()<<"OSC | "<<addr<<args;
}

JSonNode::SetError OscInterface::setValue(QString name, QString value)
{
    if(name=="address"){
        setOscAddress(value);
        return NoError;
    }else if(name=="port"){
        bool success;
        int port = value.toInt(&success);
        if(!success)
            return WrongArg;
        setPort(port);
        return NoError;
    }
    
    return JSonNode::setValue(name, value);
}

bool OscInterface::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    
    if(function == "sendTo" || function=="sendToAll"){
        int all = function=="sendToAll";
        if(args.size() < 3 - all ){
            returnCb("#Error: Wrong argument number");
            return true;
        }
        
        QString ip = "";
        if(!all)
            ip = args.at(0);
        
        bool ok = true;
        int port = args.at(1-all).toInt(&ok);
        if(!ok){
            returnCb("#Error: second argument shoud be a port");
            return true;
        }
        QString osc = args.at(2-all);
        
        QVariantList l;
        for(int i = 3-all; i<args.size(); i++){
            double a = args.at(i).toDouble(&ok);
            if(ok)
                l.append(a);
            else if(args.at(i).compare("true", Qt::CaseInsensitive))
                l.append(true);
            else if(args.at(i).compare("false", Qt::CaseInsensitive))
                l.append(false);
            else
                l.append(args.at(i));
        }
        if(all)
            qosc::sendMsg(port, osc, l);
        else if(!qosc::sendMsg(ip, port, osc, l))
            returnCb("#Error: \""+ip+"\" is not a valid network address");
            
        return true;
    }
    
    return false;
}

void OscInterface::generateHelp(bool function)
{
    if(!function){
        addHelp("address", "OSC address", false);
        addHelp("port", "OSC listening port", false);
    }else{
        addHelp("sendTo( ip, port, oscAddr, args ... )", "send an OSC event to a specific IPs", true);
        addHelp("sendToAll( port, oscAddr, args ... )", "send an OSC event to all network", true);
    }
}
