#include "misc.h"
#include "networkmodel.h"

#include "xbee/xbeeinterface.h"

NetworkModel::NetworkModel()
    :JSonModel("network")
{
    _remotes = 0;
    _patch = 0;
    initModel();
}

void NetworkModel::setXbeeUsbPort(std::string port)
{
    setString("USBPort", QString::fromStdString(port));
}


bool NetworkModel::createSubNode(QString name, const QJsonObject &data)
{
    if(name == "Remotes"){
        _remotes = new RemoteList(this);
        if(!_remotes->populateNode(data)){
            delete _remotes;
            _remotes = 0;
            return false;
        }
        return true;
    }else if(name == "Patch"){
        _patch = new Patch(this);
        if(!_patch->populateNode(data)){
            delete _patch;
            _patch = 0;
            return false;
        }
        return true;
    }

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
        for (size_t i = 0; i < ports.size(); ++i) {
            s += QString::fromStdString(ports.at(i));
            if(i!=ports.size()-1)
                s+="\n";
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

void Remote::setButtonState(Remote::FTriggerEvent b, bool pressed)
{
    QString bName;
    switch(b){
    case Remote::UP:
        bName = "BUp";
        break;
    case Remote::DOWN:
        bName = "BDown";
        break;
    case Remote::RIGHT:
        bName = "BRight";
        break;
    case Remote::LEFT:
        bName = "BLeft";
        break;
    case Remote::ACTION:
        bName = "BCenter";
        break;
    }

    setBool(bName, pressed);
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
    o.insert("BUp",false);
    o.insert("BDown",false);
    o.insert("BLeft",false);
    o.insert("BRight",false);
    o.insert("BCenter",false);
    o.insert("LED", 0);

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
    }
    return false;
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

JSonNode::SetError Remote::setValue(QString name, QString value)
{
    if(name=="LED"){
        bool success;
        uint8_t intensity = value.toInt(&success);
        if(!success)
            return WrongArg;
        if(remote())
            remote()->safeSendMsg("L",LED_INTENSITY, std::vector<uint8_t>({intensity}));

        return setNumber(name, intensity);
    }else if(name=="State"){
        if(remote()){
        if(value=="active")
            remote()->safeSendMsg("S", ACTIVE_MODE);
        else if(value == "mute")
            remote()->safeSendMsg("S", MUTE_MODE);
        else
            return JSonNode::setValue(name, value);
        }
        return JSonNode::setString(name, value);
    }

    return JSonNode::setValue(name, value);
}



