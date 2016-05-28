#include <QJsonArray>

#include "virtualnetwork.h"
#include "networkmodel.h"

VirtualNetwork::VirtualNetwork(EventModel *eventModel)
    :JSonNode("VirtualNet", eventModel)//, _eventModel(eventModel)
{

}

bool VirtualNetwork::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("slaveNbr")){
        VirtualGroup* g = new VirtualGroup(name, this);
        if(!g->populateNode(data)){
            delete g;
            return false;
        }
        _vGroups.append(g);
        return true;
    }else if(data.contains("LED")){
        VirtualRemote* r = new VirtualRemote(name, this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        _vRemotes.append(r);
        return true;
    }
    return false;
}

bool VirtualNetwork::addRemote(QString name)
{
    if(_jsonData.contains(name))
        return false;

    if(name == ""){
        name = "Remote";
        int nbr = 1;
        while(_jsonData.keys().contains(name+QString().setNum(nbr)))
            nbr++;
        name += QString().setNum(nbr);
    }

    createSubNode(name, VirtualRemote::createVirtualRemoteJSon());

    return true;
}

VirtualRemote *VirtualNetwork::remoteByName(QString name)
{
    for (int i = 0; i < _vRemotes.size(); ++i) {
        if(_vRemotes[i]->name() == name)
            return _vRemotes[i];
    }
    return 0;
}

bool VirtualNetwork::removeRemote(QString name)
{
    VirtualRemote* r = remoteByName(name);
    if(!r)
        return false;

    _vRemotes.removeOne(r);
    removeSubNode(r);
    return true;
}



bool VirtualNetwork::addGroup(QString name)
{
    if(_jsonData.contains(name))
        return false;

    if(name == ""){
        name = "Group";
        int nbr = 1;
        while(_jsonData.keys().contains(name+QString().setNum(nbr)))
            nbr++;
        name += QString().setNum(nbr);
    }

    createSubNode(name, VirtualGroup::createGroupJSon());

    return true;
}

VirtualGroup *VirtualNetwork::groupByName(QString name)
{
    for (int i = 0; i < _vGroups.size(); ++i) {
        if(_vGroups[i]->name() == name)
            return _vGroups[i];
    }
    return 0;
}

bool VirtualNetwork::removeGroup(QString name)
{
    VirtualGroup* g = groupByName(name);
    if(!g)
        return false;

    _vGroups.removeOne(g);
    removeSubNode(g);
    return true;
}

void VirtualNetwork::autoGenerateRemote()
{
    QList<Remote*>& l = SNetworkModel::ptr()->remotes()->remotes();
    foreach(Remote* r, l){
        addRemote(r->name());
    }
}

bool VirtualNetwork::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function=="addRemote"){
        if(args.size()>1){
            returnCb("#Error: Wrong arguments number, remote name expected");
            return true;
        }
        QString name = "";
        if(args.size()==1)
            name = args[0];
        if(!addRemote(name))
            returnCb("#Error: This remote name is already taken");
        return true;
    }else if(function=="removeRemote"){
        if(args.size()!=1){
            returnCb("#Error: Wrong arguments number, remote name expected");
            return true;
        }
        if(!removeRemote(args[0]))
            returnCb("#Error: no \""+args[0]+"\" remote found");
        return true;
    }else if(function=="addGroup"){
        if(args.size()>1){
            returnCb("#Error: Wrong arguments number, group name expected");
            return true;
        }
        QString name = "";
        if(args.size()==1)
            name = args[0];
        if(!addGroup(name))
            returnCb("#Error: This group name is already taken");
        return true;
    }else if(function=="removeGroup"){
        if(args.size()!=1){
            returnCb("#Error: Wrong arguments number, group name expected");
            return true;
        }
        if(!removeGroup(args[0]))
            returnCb("#Error: no \""+args[0]+"\" group found");
        return true;
    }else if(function=="autoGenerateRemote"){
        autoGenerateRemote();
        return true;
    }
    return false;
}



/********************************************
 *             Virtual Remote               *
 *******************************************/

VirtualRemote::VirtualRemote(QString name, VirtualNetwork *virtualNet)
    :JSonNode(name, virtualNet, RENAMEABLE), _vNet(virtualNet)
{
    initEventsHandler();
}

VirtualRemote::VirtualRemote(QString name, VirtualGroup *group)
    :JSonNode(name, group), _vNet(group->virtualNet()), _group(group)
{
    initEventsHandler();
}
void VirtualRemote::initEventsHandler()
{
    QStringList names;
    names <<"EventUp"<<"EventDown"<<"EventRight"<<"EventLeft"<<"EventAction";
    for(int i=0;i<names.size();i++)
        _eventsHandler.append(new Event(names[i], this, false));
}

bool VirtualRemote::createSubNode(QString name, const QJsonObject &data)
{
    if(name=="EventUp" || name=="EventDown" || name=="EventLeft" || name=="EventRight" || name=="EventAction"){

        int id = Remote::ACTION;
        if(name=="EventUp")
            id = Remote::UP;
        else if(name=="EventDown")
            id = Remote::DOWN;
        else if(name=="EventLeft")
            id = Remote::LEFT;
        else if(name=="EventRight")
            id = Remote::RIGHT;

        if(!_eventsHandler[id]->populateNode(data))
            return false;

        return true;
    }
    return false;
}

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
    o.insert("EventRight", Event::createEventJSon());
    o.insert("EventAction", Event::createEventJSon());
    return o;
}

JSonNode::SetError VirtualRemote::setValue(QString name, QString value)
{
    if(name=="Up"){
        if(_jsonData.value("Up").toString()==value)
            return JSonNode::SameValue;
        if(value=="ON")
            _eventsHandler[Remote::UP]->triggerEvent("up");
        else if(value=="OFF")
            _eventsHandler[Remote::UP]->triggerEvent("down");
        else
            return WrongArg;
        return setString(name, value);
    }else if(name=="Down"){
        if(_jsonData.value("Down").toString()==value)
            return JSonNode::SameValue;
        if(value=="ON")
            _eventsHandler[Remote::DOWN]->triggerEvent("up");
        else if(value=="OFF")
            _eventsHandler[Remote::DOWN]->triggerEvent("down");
        else
            return WrongArg;
        return setString(name, value);
    }else if(name=="Left"){
        if(_jsonData.value("Left").toString()==value)
            return JSonNode::SameValue;
        if(value=="ON")
            _eventsHandler[Remote::LEFT]->triggerEvent("up");
        else if(value=="OFF")
            _eventsHandler[Remote::LEFT]->triggerEvent("down");
        else
            return WrongArg;
        return setString(name, value);
    }else if(name=="Right"){
        if(_jsonData.value("Right").toString()==value)
            return JSonNode::SameValue;
        if(value=="ON")
            _eventsHandler[Remote::RIGHT]->triggerEvent("up");
        else if(value=="OFF")
            _eventsHandler[Remote::RIGHT]->triggerEvent("down");
        else
            return WrongArg;
        return setString(name, value);
    }else if(name=="Action"){
        if(_jsonData.value("Up").toString()==value)
            return JSonNode::SameValue;
        if(value=="ON")
            _eventsHandler[Remote::UP]->triggerEvent("up");
        else if(value=="OFF")
            _eventsHandler[Remote::UP]->triggerEvent("down");
        else
            return WrongArg;
        return setString(name, value);
    }

    return JSonNode::setValue(name, value);
}

bool VirtualRemote::execFunction(QString, QStringList, const std::function<void (QString)> &)
{
    return false;
}

/********************************************
 *                 Event                    *
 *******************************************/

Event::Event(QString name, VirtualRemote *remote, bool inEvents)
    :JSonNode(name, remote), _remote(remote), _inEvents(inEvents)
{
    _anyTrigger = new EventTrigger("any", _inEvents, this);
}

bool Event::addEvent(QString description, QString trigger)
{
    EventTrigger* e = 0;
    QJsonArray a;
    if(trigger == "any"){
        e = _anyTrigger;
        a = _jsonData["any"].toArray();
    }else if( !(e=eventByTrigger(trigger)) ){
        e = new EventTrigger(trigger, _inEvents, this);
        _triggers.insert(trigger, e);
    }else
        a = _jsonData[trigger].toArray();
    if(!e->addEvent(description))
        return false;
    a.push_back(description);
    _jsonData[trigger] = a;
    valueChanged(trigger);
    return true;
}

EventTrigger *Event::eventByTrigger(QString trigger)
{
    if(trigger=="any")
        return _anyTrigger;

    auto e = _triggers.find(trigger);
    if(e!=_triggers.end()){
        e.value()->trigger();
        return e.value();
    }
    return 0;
}

bool Event::removeEvent(QString definition, QString trigger)
{
    EventTrigger* e;
    if(trigger == "any")
        e = _anyTrigger;
    else if(! (e=eventByTrigger(trigger)) )
        return false;
    if(!e->removeEvent(definition))
        return false;

    QJsonArray a =_jsonData[trigger].toArray();
    QJsonArray::iterator it;
    for(it = a.begin(); it!=a.end(); it++){
        if(it->toString()==definition){
            _jsonData[trigger].toArray().erase(it);
            break;
        }
    }
    if(it==a.end())
        return false;

    valueChanged(trigger);
    return true;
}

QStringList Event::triggers() const {
    QStringList r = _jsonData.keys();
    r.prepend("any");
    return r;
}

bool Event::triggerEvent(QString trigger)
{
    _anyTrigger->trigger();
    auto e = _triggers.find(trigger);
    if(e!=_triggers.end()){
        e.value()->trigger();
        return true;
    }
    return false;
}

bool Event::isInEvents() const {return _inEvents;}

VirtualRemote *Event::remote() {return _remote;}

QJsonObject Event::createEventJSon()
{
    QJsonObject o;
    o.insert("any", QJsonArray());
    return o;
}

bool Event::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function == "add" || function=="remove"){
        if(args.size()<1){
            returnCb("#Error: Wrong arguments number, expected event definition");
            return false;
        }
        QString trigger = "any";
        QString definition = args[0];
        if(args.size()>1)
            trigger = args[1];

        if(function == "add")
            addEvent(definition, trigger);
        else{
            if(!removeEvent(definition, trigger)){
                returnCb("#Error: \""+definition+"\" not found for \""+trigger+"\" trigger.");
                return false;
            }
        }
        return true;
    }


    return false;
}

JSonNode::SetError Event::parseArray(QString name, QStringList value)
{
    foreach (QString v, value) {
        addEvent(v, name);
    }
    return NoError;
}

/********************************************
 *                 Group                    *
 *******************************************/

VirtualGroup::VirtualGroup(QString name, VirtualNetwork *virtualNet)
    :JSonNode(name, virtualNet, RENAMEABLE), _vNet(virtualNet)
{

}

VirtualNetwork *VirtualGroup::virtualNet() {return _vNet;}

void VirtualGroup::setSlavesNbr(int nbr)
{
    setNumber("slaveNbr", nbr);
}

VirtualRemote *VirtualGroup::globalRemote() {return _globalRemote;}

void VirtualGroup::setVirtualRemotesNbr(int nbr)
{
    int delta = nbr-_jsonData.count()+2;

    while(delta!=0){
        if(delta>0){
            QString rName = "SRemote"+QString().setNum(nbr-delta+1);
            createSubNode(rName, VirtualRemote::createVirtualRemoteJSon());
            delta--;
        }else{
            removeSubNode(_virtualRemotes.last());
            _virtualRemotes.removeLast();
            delta++;
        }
    }
}

VirtualRemote *VirtualGroup::virtualRemote(int id)
{
    if(id >= _virtualRemotes.size())
        return 0;
    return _virtualRemotes.at(id);
}

QJsonObject VirtualGroup::createGroupJSon()
{
    QJsonObject o;
    o.insert("slaveNbr", 0);
    o.insert("vRemotesNbr", 0);
    o.insert("GlobalRemote", VirtualRemote::createVirtualRemoteJSon());
    return o;
}

bool VirtualGroup::createSubNode(QString name, const QJsonObject &data)
{
    if(name=="GlobalRemote" || name.startsWith("Remote")){
        VirtualRemote* r = new VirtualRemote(name, this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        if(name[0]=='G')
            _globalRemote = r;
        else{
            bool success=false;
            int remoteID = name.mid(5).toInt(&success);
            if(!success){
                delete r;
                return false;
            }
            while(_virtualRemotes.count()<=remoteID)
                _virtualRemotes.append(0);
            _virtualRemotes[remoteID] = r;
        }
        return true;
    }
    return false;
}

JSonNode::SetError VirtualGroup::setValue(QString name, QString value)
{
    if(name=="slaveNbr"){
        return setNumber(name, value);
    }else if(name == "vRemotesNbr"){
        bool success = false;
        int nbr = value.toInt(&success);
        setVirtualRemotesNbr(nbr);
        return setNumber(value, nbr);
    }

    return JSonNode::setValue(name, value);
}

bool VirtualGroup::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    return false;
}


