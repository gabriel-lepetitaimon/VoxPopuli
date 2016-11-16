#include "misc.h"
#include "networkmodel.h"

#include "xbee/xbeeinterface.h"
#include "qosc.h"

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

    createSubNode(remoteName, RealRemote::createRealRemoteJSon(address));

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

bool RemoteList::emulateRemote(QString name, QString osc, QString ipPort)
{
    foreach (Remote* r, _remotes) {
        if(r->name() == name)
            return false;
        if(!osc.isEmpty() && r->get("osc").toString() == osc)
            return false;
    }

    createSubNode(name, EmulatedRemote::createEmulatedRemoteJSon(osc, ipPort));

    return true;
}

bool RemoteList::removeEmulatedRemote(QString name)
{
    for (int i = 0; i < _remotes.size(); ++i) {
        if(_remotes.at(i)->name() == name){
            if(!dynamic_cast<EmulatedRemote*>(_remotes.at(i)) )
                return false;
            removeSubNode(_remotes.at(i));
            _remotes.removeAt(i);
            return true;
        }
    }
    return false;
}

bool RemoteList::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("MAC")){
        RealRemote* r = new RealRemote(name, data.value("MAC").toString(), this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        _remotes.append(r);
        return true;
    }else if(data.contains("osc")){
        EmulatedRemote* r = new EmulatedRemote(name, this, data.value("osc").toString());
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        _remotes.append(r);
        return true;
    }
    return false;
}

bool RemoteList::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function == "list"){
        QString strList;
        foreach (Remote* r, _remotes){
            QString addr = r->get("MAC").toString();
            if(addr.isEmpty())
                addr = r->get("osc").toString();
            strList+= r->name() + " " + addr + "\n";
        }
        strList.remove(strList.size()-1);
        returnCb(strList);
        return true;
    }else if(function=="emulateRemote"){
        if(args.size()>2 || args.size()==0){
            returnCb("#Error: Wrong arguments number, remote name and osc address expected (osc address is optional)");
            return true;
        }
            QString name = args[0], osc ="";
            if(args.size()==2)
                osc = args[1];
        if(!emulateRemote(name, osc))
            returnCb("#Error: This remote name or osc address is already taken");
        return true;
    }else if(function=="removeEmulatedRemote"){
        if(args.size()!=1){
            returnCb("#Error: Wrong arguments number, remote name expected ");
            return true;
        }
        if(!removeEmulatedRemote(args[0]))
            returnCb("#Error: No emulated remote is named \""+args[0]+"\".");
        return true;
    }
    return false;
}

void RemoteList::generateHelp(bool function)
{
    if(function){
    addHelp("list()", "List connected remotes.", true);
    addHelp("emulateRemote( name, osc )", "Add an emulated remote (osc address is optionnal).", true);
    addHelp("removeEmulatedRemote( name )", "Remove an emulated remote.", true);
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

RealRemote *RemoteList::byMAC(QString mac)
{
    for(auto it = _remotes.begin(); it!=_remotes.end(); it++){
        RealRemote *r = dynamic_cast<RealRemote*>(*it);
        if(r && r->macAddress()==mac)
            return r;
    }
    return 0;
}

EmulatedRemote *RemoteList::byOSC(QString osc)
{
    for(auto it = _remotes.begin(); it!=_remotes.end(); it++){
        EmulatedRemote *r = dynamic_cast<EmulatedRemote*>(*it);
        if(r && r->oscAddress()==osc)
            return r;
    }
    return 0;
}

QStringList RemoteList::remotesNames() const
{
    QStringList l;
    foreach(Remote* r, _remotes)
        l.append(r->name());
    return l;
}


/********************************************
 *              Remote                      *
 *******************************************/

Remote::Remote(QString name, RemoteList *list)
    :JSonNode(name, list, RENAMEABLE)
{
}

Remote::~Remote()
{
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


QJsonObject Remote::createRemoteJSon()
{
    QJsonObject o;
    o.insert("State", "Initializing");
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
    if(function == "LED"){
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
    addHelp("State","Current remote's state ",false);
    addHelp("Up", "State of the UP button of the remote (READONLY)", false);
    addHelp("Down", "State of the DOWN button of the remote (READONLY)", false);
    addHelp("Left", "State of the LEFT button of the remote (READONLY)", false);
    addHelp("Right", "State of the RIGHT button of the remote (READONLY)", false);
    addHelp("Action", "State of the ACTION button of the remote (READONLY)", false);
    addHelp("LED1", "Intensity of the LED1", false);
    addHelp("LED2", "Intensity of the LED2", false);
    addHelp("LED3", "Intensity of the LED3", false);
    }else{
    addHelp("LED( intensity )", "Set the intensity of the three LEDs", true);
    }
}


JSonNode::SetError Remote::fastTrigger(FTriggerEvent e, const HexData &v)
{
    if(e==LED){
        setNumber("LED1", v.toInt());
        setNumber("LED2", v.toInt());
        setNumber("LED3", v.toInt());
        return NoError;
    }else if(e==LED1){
        return setNumber("LED1", v.toInt());
    }else if(e==LED2){
        return setNumber("LED2", v.toInt());
    }else if(e==LED3){
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
        return JSonNode::setString(name, value);
    }

    return JSonNode::setValue(name, value);
}

/********************************************
 *              Real Remote                 *
 *******************************************/

RealRemote::RealRemote(QString name, QString mac, RemoteList *list)
    :Remote(name, list)
{
        _jsonData["MAC"] = mac;
}

QJsonObject RealRemote::createRealRemoteJSon( QString address)
{
    QJsonObject o = Remote::createRemoteJSon();
    o.insert("MAC", address);
    o.insert("SignalStrength", "NA");
    o.insert("Battery", "HIGH");

    return o;
}

RealRemote::~RealRemote()
{
    if(remote())
        remote()->clearRemoteModel();
}

XBeeRemote *RealRemote::remote()
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

void RealRemote::setSignalStrength(int dB)
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

bool RealRemote::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
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
    }

    return Remote::execFunction(function, args, returnCb);
}

void RealRemote::generateHelp(bool function)
{
    if(!function){
    addHelp("MAC","MAC address of the remote's Xbee. (READONLY)",false);
    addHelp("SignalStrength","Signal strength of the remote (READONLY)",false);
    addHelp("Battery","Battery level of the remote (READONLY)",false);
    }else{
    addHelp("sendAT( atCmd )", "Send an AT command to the remote's XBee", true);
    addHelp("sendTX( cmd )", "Send a command to the remote's arduino", true);
    addHelp("sendTXHex( cmd )", "Send a command to the remote's arduino", true);
    }

    Remote::generateHelp(function);
}

JSonNode::SetError RealRemote::setValue(QString name, QString value)
{
    if(name=="State"){
        if(remote()){
            if(value=="active")
                remote()->safeSendMsg("S", ACTIVE_MODE);
            else if(value == "mute")
                remote()->safeSendMsg("S", MUTE_MODE);
        }
        return JSonNode::setString(name, value);
    }

    return Remote::setValue(name, value);
}

JSonNode::SetError RealRemote::fastTrigger(FTriggerEvent e, const HexData &v)
{
    if(e==LED){
        if(remote())
            remote()->safeSendMsg("L",LED_INTENSITY, v);
    }else if(e==LED1){
        if(remote())
            remote()->safeSendMsg("L1",LED1_INTENSITY, v);
    }else if(e==LED2){
        if(remote())
            remote()->safeSendMsg("L2",LED2_INTENSITY, v);
    }else if(e==LED3){
        if(remote())
            remote()->safeSendMsg("L3",LED3_INTENSITY, v);
    }

    return Remote::fastTrigger(e,v);
}

/********************************************
 *           Emulated Remote                *
 *******************************************/

EmulatedRemote::EmulatedRemote(QString name, RemoteList *list, QString oscAddress, QString ipPort)
    :Remote(name, list)
{
    _jsonData["osc"] = oscAddress;
    _jsonData["ipPort"] = ipPort;
}

QJsonObject EmulatedRemote::createEmulatedRemoteJSon( QString oscAddress, QString ipPort)
{
    QJsonObject o = Remote::createRemoteJSon();
    o.insert("osc", oscAddress);
    o.insert("ipPort", ipPort);
    return o;
}

EmulatedRemote::~EmulatedRemote()
{
}

QString EmulatedRemote::forwardIP() const
{
    QString ipPort = _jsonData["ipPort"].toString();
    if(ipPort.count(':')!=1);
        return "";
    return ipPort.split(':').first();
}

int EmulatedRemote::forwardPort() const
{
    QString ipPort = _jsonData["ipPort"].toString();
    if(ipPort.count(':')!=1)
        return -1;
    return ipPort.split(':').last().toInt();
}

void EmulatedRemote::readOsc(QVariantList args)
{
    if(args.size()<2 || args.at(0).type()!=QVariant::String || args.at(1).type()!=QVariant::Bool)
        return;
    
    bool state = args.at(1).toBool();
    if(args.at(0).toString()=="up")
        setButtonState(BUTTON_UP, state);
    else if(args.at(0).toString()=="down")
        setButtonState(BUTTON_DOWN, state);
    else if(args.at(0).toString()=="left")
        setButtonState(BUTTON_LEFT, state);
    else if(args.at(0).toString()=="right")
        setButtonState(BUTTON_RIGHT, state);
    else if(args.at(0).toString()=="action")
        setButtonState(BUTTON_ACTION, state);
}

bool EmulatedRemote::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    return Remote::execFunction(function, args, returnCb);
}

void EmulatedRemote::generateHelp(bool function)
{
    if(function){
    }else{
        addHelp("osc", "OSC address of this emulated remote (optionnal)", true);
        addHelp("ipPort", "OSC ip and port to where LED changes should be forwarded (the format should be \"ip:port\")", true);
    }

    Remote::generateHelp(function);
}

JSonNode::SetError EmulatedRemote::setValue(QString name, QString value)
{
    XBEE_MSG_TYPE type = FRAME_END;
    if(name=="Up")
        type = BUTTON_UP;
    else if(name=="Down")
        type = BUTTON_DOWN;
    else if(name == "Left")
        type = BUTTON_LEFT;
    else if(name == "Rigth")
        type = BUTTON_RIGHT;
    else if(name == "Action")
        type = BUTTON_ACTION;
    else if(name == "osc"){
        setString(name, value);
        return NoError;
    }else if(name == "ipPort"){
         if(value.count(':')!=1)
             return WrongArg;
         setString("ipPort", value);
         return NoError;
    }else
        return Remote::setValue(name, value);

    if(value.toLower()=="up" || value=="0")
        setButtonState(type, 0);
    else if(value.toLower() == "down" || value == "1")
        setButtonState(type, 1);
    else
        return WrongArg;

    return NoError;
}

JSonNode::SetError EmulatedRemote::fastTrigger(FTriggerEvent e, const HexData &v)
{
    //Todo: OSC trigger...
    if(forwardPort()!=-1){
        QVariantList args;
        
        bool skip = false;
        if(e==LED){
            args<<"LED";
        }else if(e==LED1){
            args<<"LED1";
        }else if(e==LED2){
            args<<"LED2";
        }else if(e==LED3){
            args<<"LED3";
        }else
            skip = true;
        
        args<<v.toInt();
        if(!skip)
            qosc::sendMsg(forwardIP(), forwardPort(), oscAddress(), args);
    }

    return Remote::fastTrigger(e,v);
}
