#include "misc.h"
#include "networkmodel.h"

#include "xbee/xbeeinterface.h"

NetworkModel::NetworkModel()
    :JSonModel("network")
{
    _remotes = new RemoteList(this);
    _patch = new Patch(this);
}

void NetworkModel::setXbeeUsbPort(std::string port)
{
    setString("USBPort", QString::fromStdString(port));
}


bool NetworkModel::createSubNode(QString name, const QJsonObject &data)
{
    if(name == "Remotes")
        return _remotes->populateNode(data);
    else if(name == "Patch")
        return _patch->populateNode(data);

    return false;
}

JSonNode::SetError NetworkModel::setValue(QString name, QString value)
{
    if(name=="USBPort"){
        SXBeeInterface::ptr()->forcePort(value.toStdString());
        return setString(name, value);
    }

    return JSonNode::setValue(name, value);
}

bool NetworkModel::execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb)
{
    if(function == "sendAT"){
        if(args.size()!=1)
            return false;
        SXBeeInterface::ptr()->sendAT(args.first().toStdString(),
                                      [returnCb, args](std::vector<uint8_t> v)->int{
                                                    returnCb("["+args.first().left(2)+"] "+QString::fromStdString(intToHexStr(v)));
                                                    return true;
                                        });
        returnCb("");
        return true;
    }else if(function == "listPort"){
        std::vector<std::string> ports = SXBeeInterface::ptr()->listPort();
        QString s;
        if(!ports.size())
            s = "None";
        else{
            for (size_t i = 0; i < ports.size(); ++i) {
                s += QString::fromStdString(ports.at(i));
                if(i!=ports.size()-1)
                    s+="\n";
            }
        }
        returnCb(s);
        return true;
    }else if(function == "scan"){
        returnCb("");
        SXBeeInterface::ptr()->scanNetwork();
        return true;
    }
    return false;
}

void NetworkModel::generateHelp(bool function)
{
    if(!function){
    addHelp("USBPort", "Defines antenna USB port", false);
    addHelp("Remotes", "Connected remotes", false);
    addHelp("Patch", "Connexion patch between remotes and virtual remotes", false);
    }else{
    addHelp("sendAT( atCmd )", "Send an AT command to the connected XBee Card", true);
    addHelp("listPort()", "List USB port", true);
    addHelp("scan()", "Start a scan on the XBee network to discover remotes", true);
    }
}


/********************************************
 *              RemoteList                  *
 *******************************************/

RemoteList::RemoteList(NetworkModel *model)
    :JSonNode("Remotes", model)
{

}

bool RemoteList::addRemote(QString address)
{
    foreach (Remote* r, _remotes) {
        if(r->get("MAC").toString() == address)
            return false;
    }

    QString remoteName = "Remote";
    int nbr = 1;
    while(_jsonData.keys().contains(remoteName+QString().setNum(nbr)))
        nbr++;
    remoteName += QString().setNum(nbr);

    createSubNode(remoteName, Remote::createRemoteJSon(address));

    return true;
}

void RemoteList::removeRemote(QString address)
{
    for (int i = 0; i < _remotes.size(); ++i) {
        if(_remotes[i]->get("MAC").toString() == address){
            removeSubNode(_remotes.at(i));
            _remotes.removeAt(i);
            return;
        }
    }
}

bool RemoteList::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("MAC")){
        Remote* r = new Remote(name, data.value("MAC").toString(), this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        _remotes.append(r);
        return true;
    }
    return false;
}

bool RemoteList::execFunction(QString function, QStringList , const std::function<void (QString)> &returnCb)
{
    if(function == "list"){
        QString strList;
        foreach (Remote* r, _remotes)
            strList+= r->name() + " " + r->get("MAC").toString() + "\n";
        strList.remove(strList.size()-1);
        returnCb(strList);
        return true;
    }
    return false;
}

void RemoteList::generateHelp(bool function)
{
    if(!function){
    addHelp("list()", "List connected remotes.", false);
    }
}

Remote *RemoteList::byName(QString name)
{
    for(auto it = _remotes.begin(); it!=_remotes.end(); it++){
        if((*it)->name()==name)
            return (*it);
    }
    return 0;
}

Remote *RemoteList::byAddr(QString addr)
{
    for(auto it = _remotes.begin(); it!=_remotes.end(); it++){
        if((*it)->macAddress()==addr)
            return (*it);
    }
    return 0;
}


/********************************************
 *              Remote                      *
 *******************************************/

Remote::Remote(QString name, QString mac, RemoteList *list)
    :JSonNode(name, list, RENAMEABLE)
{
    _jsonData["MAC"] = mac;
}

Remote::~Remote()
{
    if(remote())
        remote()->clearRemoteModel();
}

void Remote::setButtonState(XBEE_MSG_TYPE b, bool pressed)
{
    FTriggerEvent e;
    QString bName;
    switch(b){
    case BUTTON_UP:
        bName = "Up";
        e=UP;
        break;
    case BUTTON_DOWN:
        bName = "Down";
        e=DOWN;
        break;
    case BUTTON_RIGHT:
        bName = "Right";
        e=RIGHT;
        break;
    case BUTTON_LEFT:
        bName = "Left";
        e=LEFT;
        break;
    case BUTTON_ACTION:
        bName = "Action";
        e=ACTION;
        break;
    default:
        return;
    }

     fastTrigger(e, pressed?1:0);
    setString(bName, pressed?"down":"up");
}

void Remote::setSignalStrength(int dB)
{
    QString sStrength = "Very Low";
    if(dB > -37)
        sStrength = "Very high";
    else if(dB > -45)
        sStrength = "High";
    else if(dB > -65)
        sStrength = "Medium";
    else if(dB > -80)
        sStrength = "Low";

    setString("SignalStrength", sStrength);
}

QJsonObject Remote::createRemoteJSon( QString address)
{
    QJsonObject o;
    o.insert("MAC", address);
    o.insert("State", "Initializing");
    o.insert("SignalStrength", "NA");
    o.insert("Battery", "HIGH");
    o.insert("Up","up");
    o.insert("Down","up");
    o.insert("Left","up");
    o.insert("Right","up");
    o.insert("Action","up");
    o.insert("LED1", 0);
    o.insert("LED2", 0);
    o.insert("LED3", 0);


    return o;
}

bool Remote::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function == "sendAT"){
        if(args.size()!=1)
            return false;
        if(remote())
            remote()->sendAT(args.first().toStdString(),
                                      [returnCb, args, this](std::vector<uint8_t> v)->bool{
                                                    returnCb("["+name()+"|"+args.first().left(2)+"] "+QString::fromStdString(intToHexStr(v)));
                                                    return true;
                                        });
        return true;
    }else if(function == "sendTX"){
        if(args.size()!=1)
            return false;
        remote()->sendTX(args.first().toStdString());
        return true;
    }else if(function == "sendTXHex"){
        if(args.size()!=1)
            return false;
        std::string data;
        std::vector<uint8_t> hex = hexStrToInt(args.first().toStdString());
        for(size_t i=0; i<hex.size(); i++)
            data+=hex[i];
        remote()->sendTX(data);
        return true;
    }else if(function == "LED"){
        if(args.size()!=1)
            return false;
        bool success;
        uint8_t intensity = args[0].toInt(&success);
        if(!success)
            return false;
        fastTrigger(LED, intensity);
        return true;
    }
    return false;
}

void Remote::generateHelp(bool function)
{
    if(!function){
    addHelp("MAC","MAC address of the remote's Xbee. (READONLY)",false);
    addHelp("State","Current remote's state ",false);
    addHelp("SignalStrength","Signal strength of the remote (READONLY)",false);
    addHelp("Battery","Battery level of the remote (READONLY)",false);
    addHelp("Up", "State of the UP button of the remote (READONLY)", false);
    addHelp("Down", "State of the DOWN button of the remote (READONLY)", false);
    addHelp("Left", "State of the LEFT button of the remote (READONLY)", false);
    addHelp("Right", "State of the RIGHT button of the remote (READONLY)", false);
    addHelp("Action", "State of the ACTION button of the remote (READONLY)", false);
    addHelp("LED1", "Intensity of the LED1", false);
    addHelp("LED2", "Intensity of the LED2", false);
    addHelp("LED3", "Intensity of the LED3", false);
    }else{
    addHelp("sendAT( atCmd )", "Send an AT command to the remote's XBee", true);
    addHelp("sendTX( cmd )", "Send a command to the remote's arduino", true);
    addHelp("sendTXHex( cmd )", "Send a command to the remote's arduino", true);
    addHelp("LED( intensity )", "Set the intensity of the three LEDs", true);
    }
}

XBeeRemote *Remote::remote()
{
    if(_remote)
        return _remote;

    for (size_t i = 0; i < SXBeeInterface::ptr()->remotes().size(); ++i) {
        std::string addr = intToHexStr(SXBeeInterface::ptr()->remotes().at(i).address());
        if(QString::fromStdString(addr)==get("MAC").toString())
               return _remote = &SXBeeInterface::ptr()->remotes()[i];
    }
    if(SNetworkModel::ptr())
        SNetworkModel::ptr()->remotes()->removeRemote(get("MAC").toString());
    return 0;
}

JSonNode::SetError Remote::fastTrigger(FTriggerEvent e, const HexData &v)
{
    if(e==LED){
        if(remote())
            remote()->safeSendMsg("L",LED_INTENSITY, v);
        setNumber("LED1", v.toInt());
        setNumber("LED2", v.toInt());
        setNumber("LED3", v.toInt());
        return NoError;
    }else if(e==LED1){
        if(remote())
            remote()->safeSendMsg("L1",LED1_INTENSITY, v);
        return setNumber("LED1", v.toInt());
    }else if(e==LED2){
        if(remote())
            remote()->safeSendMsg("L2",LED2_INTENSITY, v);
        return setNumber("LED2", v.toInt());
    }else if(e==LED3){
        if(remote())
            remote()->safeSendMsg("L3",LED3_INTENSITY, v);
        return setNumber("LED3", v.toInt());
    }else{
        SNetworkModel::ptr()->patch()->fastTriggerRemote(_name, e, v);
        return NoError;
    }

    return WrongArg;
}

JSonNode::SetError Remote::setValue(QString name, QString value)
{
    if(name=="LED1"){
        bool success;
        uint8_t intensity = value.toInt(&success);
        if(!success)
            return WrongArg;
        return fastTrigger(LED1, intensity);
    }else if(name=="LED2"){
        bool success;
        uint8_t intensity = value.toInt(&success);
        if(!success)
            return WrongArg;
        return fastTrigger(LED2, intensity);
    }else if(name=="LED3"){
        bool success;
        uint8_t intensity = value.toInt(&success);
        if(!success)
            return WrongArg;
        return fastTrigger(LED3, intensity);
    }else if(name=="State"){
        if(remote()){
            if(value=="active")
                remote()->safeSendMsg("S", ACTIVE_MODE);
            else if(value == "mute")
                remote()->safeSendMsg("S", MUTE_MODE);
        }
        return JSonNode::setString(name, value);
    }


    return JSonNode::setValue(name, value);
}



