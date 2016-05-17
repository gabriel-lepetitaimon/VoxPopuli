#include "misc.h"
#include "networkmodel.h"

#include "xbee/xbeeinterface.h"

NetworkModel::NetworkModel()
    :JSonModel("network")
{
    _remotes = 0;
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
                                                    return 0;
                                        });
        returnCb("");
        return true;
    }else if(function == "listPort"){
        std::vector<std::string> ports = SXBeeInterface::ptr()->listPort();
        QString s;
        for (int i = 0; i < ports.size(); ++i) {
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
    emit( out(remoteName + " connected") );
    return createSubNode(remoteName, Remote::createRemoteJSon(address));
}

void RemoteList::removeRemote(QString address)
{
    for (int i = 0; i < _remotes.size(); ++i) {
        if(_remotes[i]->address() == address){
            _jsonData.remove(_remotes[i]->name());
            emit( out(_remotes[i]->name() + " disconnected") );
            _remotes.removeAt(i);
            return;
        }
    }
}

bool RemoteList::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("MAC")){
        Remote* r = new Remote(name, this);
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
        foreach (Remote* r, _remotes)
            strList+= r->name() + " " + r->get("MAC").toString() + "\n";
        strList.remove(strList.size()-1);
        returnCb(strList);
        return true;
    }
    return false;
}


/********************************************
 *              Remote                  *
 *******************************************/

Remote::Remote(QString name, RemoteList *list)
    :JSonNode(name, list)
{

}

void Remote::setButtonState(Remote::Button b, bool pressed)
{
    QString bName;
    switch(b){
    case Remote::TOP:
        bName = "BUp";
        break;
    case Remote::BOTTOM:
        bName = "BDown";
        break;
    case Remote::RIGHT:
        bName = "BRight";
        break;
    case Remote::LEFT:
        bName = "BLeft";
        break;
    case Remote::CENTER:
        bName = "BCenter";
        break;
    }

    setBool(bName, pressed);
}

QJsonObject Remote::createRemoteJSon( QString address)
{
    QJsonObject o;
    o.insert("MAC", address);
    o.insert("State", "Initializing");
    o.insert("Battery", "HIGH");
    o.insert("BUp","false");
    o.insert("BDown","false");
    o.insert("BLeft","false");
    o.insert("BRight","false");
    o.insert("BCenter","false");
    o.insert("LED", "0");

    return o;
}

bool Remote::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function == "sendAT"){
        if(args.size()!=1)
            return false;
        remote()->sendAT(args.first().toStdString(),
                                      [returnCb, args, this](std::vector<uint8_t> v)->int{
                                                    returnCb("["+name()+"|"+args.first().left(2)+"] "+QString::fromStdString(intToHexStr(v)));
                                                    return 0;
                                        });
        returnCb("");
        return true;
    }if(function == "sendRX"){
        if(args.size()!=1)
            return false;
        remote()->sendRX(args.first().toStdString());
        returnCb("");
        return true;
    }
    return false;
}

XBeeRemote *Remote::remote()
{
    if(_remote)
        return _remote;

    for (int i = 0; i < SXBeeInterface::ptr()->remotes().size(); ++i) {
        std::string addr = intToHexStr(SXBeeInterface::ptr()->remotes().at(i).address());
        if(QString::fromStdString(addr)==get("MAC").toString())
               return _remote = &SXBeeInterface::ptr()->remotes()[i];
    }

    SNetworkModel::ptr()->remotes()->removeRemote(get("MAC").toString());
    return 0;
}
