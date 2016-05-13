#include "networkmodel.h"
#include "xbee/xbeeinterface.h"

NetworkModel::NetworkModel()
    :JSonModel("network")
{
    initModel();
}

void NetworkModel::setXbeeUsbPort(std::string port)
{
    setString("USBPort", QString::fromStdString(port));
}

bool NetworkModel::createSubNode(QString name, QJsonValueRef data)
{
    if(name == "Remotes"){
        RemoteList* r = new RemoteList(this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        return true;

    }


}

JSonNode::SetError NetworkModel::setValue(QString name, QString value)
{
    if(name=="USBPort"){
        SXBeeInterface::ptr()->forcePort(value.toStdString());
        return setString(name, value);
    }

    return DoesNotExist;
}

/********************************************
 *              RemoteList                  *
 *******************************************/

RemoteList::RemoteList(NetworkModel *model)
    :JSonNode("Remotes", model)
{

}


bool RemoteList::createSubNode(QString name, QJsonValueRef data)
{
    return false;
}


/********************************************
 *              Remote                  *
 *******************************************/

Remote::Remote(QString name, RemoteList *list)
    :JSonNode(name, list)
{

}

QJsonObject Remote::generateNode( QString address)
{
    QJsonObject o;
    o.insert("IP", address);
    o.insert("State", "Initializing");
    o.insert("Battery", "HIGH");

    return o;
}

JSonNode::SetError Remote::setValue(QString , QString )
{

    return DoesNotExist;
}
