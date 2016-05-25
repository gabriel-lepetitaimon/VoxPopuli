#include "virtualnetwork.h"

VirtualNetwork::VirtualNetwork(EventModel *eventModel)
    :JSonNode("VirtualNet", eventModel), _eventModel(eventModel)
{

}

bool VirtualNetwork::createSubNode(QString , const QJsonObject &)
{
    return false;
}

VirtualRemote *VirtualNetwork::addRemote(QString name)
{

}

bool VirtualNetwork::removeRemote(QString name)
{

}

Group *VirtualNetwork::addGroup(QString name)
{

}

bool VirtualNetwork::removeGroup(QString name)
{

}

void VirtualNetwork::autoGenerateRemote()
{

}

bool VirtualNetwork::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{

}



/********************************************
 *             Virtual Remote               *
 *******************************************/

VirtualRemote::VirtualRemote(QString name, VirtualNetwork *virtualNet)
    :JSonNode(name, virtualNet, RENAMEABLE), _vNet(virtualNet)
{

}

VirtualNetwork *VirtualRemote::virtualNet() {return _vNet;}

QJsonObject VirtualRemote::createVirtualRemoteJSon()
{
    QJsonObject o;
    o.insert("LED", 0);
    o.insert("Up", "OFF");
    o.insert("Down", "OFF");
    o.insert("Left", "OFF");
    o.insert("Right", "OFF");
    o.insert("Action", "OFF");
    o.insert("State", "active");
    
    o.insert("EventUp", Event::createEventJSon());
    o.insert("EventDown", Event::createEventJSon());
    o.insert("EventLeft", Event::createEventJSon());
    o.insert("EventRigth", Event::createEventJSon());
    o.insert("EventAction", Event::createEventJSon());
    return o;
}

JSonNode::SetError VirtualRemote::setValue(QString name, QString value)
{

}

bool VirtualRemote::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{

}

/********************************************
 *                 Event                    *
 *******************************************/

Event::Event(VirtualRemote *remote, bool eventIn)
{

}

void Event::addEvent(QString trigger, QString description)
{

}

void Event::removeEvent(QString trigger, QString description)
{

}

void Event::removeEvent(QString trigger, int id)
{

}

int Event::eventNbr(QString trigger) const
{

}

QStringList Event::triggers() const {return _jsonData.keys();}

bool Event::triggerEvent(QString trigger)
{

}

bool Event::isEventIn() const {return _eventIn;}

VirtualRemote *Event::remote() {return _remote;}

QJsonObject Event::createEventJSon()
{
    QJsonObject o;
    return o;
}

bool Event::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{

}

/********************************************
 *                 Group                    *
 *******************************************/

Group::Group(QString name, VirtualNetwork *virtualNet)
    :JSonNode(name, virtualNet, RENAMEABLE), _vNet(virtualNet)
{

}

VirtualNetwork *Group::virtualNet() {return _vNet;}

void Group::setSlavesNbr(int nbr)
{

}

VirtualRemote *Group::globalRemote() {return _globalRemote;}

void Group::setSpecialRemotesNbr(int nbr)
{

}

int Group::getSpecialRemoteNbr() const {return _specialRemotes.size();}

VirtualRemote *Group::specialRemote(int id)
{

}

QJsonObject Group::createGroupJSon()
{
    QJsonObject o;
    o.insert("slaveNbr", 0);
    o.insert("specialRemoteNbr", 0);
    o.insert("GlobalRemote", VirtualRemote::createVirtualRemoteJSon());
}

JSonNode::SetError Group::setValue(QString name, QString value)
{

}

bool Group::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{

}


