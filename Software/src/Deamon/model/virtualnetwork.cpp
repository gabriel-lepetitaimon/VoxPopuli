#include <QJsonArray>

#include "virtualnetwork.h"
#include "networkmodel.h"

VirtualNetwork::VirtualNetwork(EventModel *eventModel)
    :JSonNode("VirtualNet", eventModel)//, _eventModel(eventModel)
{

}

bool VirtualNetwork::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("GlobalRemote")){
        VirtualGroup* g = new VirtualGroup(name, this);
        if(!g->populateNode(data)){
            delete g;
            return false;
        }
        _vGroups.append(g);
        g->updateFromPatch();
        return true;
    }else if(data.contains("LED")){
        VirtualRemote* r = new VirtualRemote(name, this);
        if(!r->populateNode(data)){
            delete r;
            return false;
        }
        _vRemotes.append(r);
        r->updateVirtualRemote();
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

void VirtualNetwork::fastTrigger(QStringList virtualElements, QString remoteName, FTriggerEvent e, const HexData &data)
{
    foreach(QString elementName, virtualElements){
        bool found = false;
        for (int i = 0; i < _vGroups.size(); ++i) {
            if(_vGroups.at(i)->name()==elementName){
                found = true;
                _vGroups.at(i)->fastTrigger(remoteName, e, data);
                break;
            }
        }
        if(found)
            continue;
        for (int i = 0; i < _vRemotes.size(); ++i) {
            if(_vRemotes.at(i)->name()==elementName){
                found = true;
                _vRemotes.at(i)->fastTrigger( e, data);
                break;
            }
        }
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

void VirtualNetwork::generateHelp(bool function)
{
    if(!function){
        addHelp("GlobalRemote", "A remote to control them all", false);
    }else{
        addHelp("addRemote( name )", "Add a virtual remote", true);
        addHelp("removeRemote( name )", "Remove a virtual remote", true);
        addHelp("addGroup( name )", "Add a virtual group", true);
        addHelp("removeGroup( name )", "Remove a virtual group", true);
        addHelp("autoGenerateRemote()", "Auto generate virtuals remotes according to real remotes", true);
    }
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

        int id = ACTION;
        if(name=="EventUp")
            id = UP;
        else if(name=="EventDown")
            id = DOWN;
        else if(name=="EventLeft")
            id = LEFT;
        else if(name=="EventRight")
            id = RIGHT;

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
    o.insert("Up", "NA");
    o.insert("Down", "NA");
    o.insert("Left", "NA");
    o.insert("Right", "NA");
    o.insert("Action", "NA");
    o.insert("State", "active");

    o.insert("EventUp", Event::createEventJSon());
    o.insert("EventDown", Event::createEventJSon());
    o.insert("EventLeft", Event::createEventJSon());
    o.insert("EventRight", Event::createEventJSon());
    o.insert("EventAction", Event::createEventJSon());
    return o;
}

void VirtualRemote::updateVirtualRemote()
{
    QList<Remote*> remotes = patchedRemotes();
    if(remotes.isEmpty()){
        // ---  RESET  ---
        setString("Up",     "NA");
        setString("Down",   "NA");
        setString("Left",   "NA");
        setString("Right",  "NA");
        setString("Action", "NA");
    }else{
        setString("Up", remotes.first()->get("Up").toString());
        setString("Down", remotes.first()->get("Down").toString());
        setString("Left", remotes.first()->get("Left").toString());
        setString("Right", remotes.first()->get("Right").toString());
        setString("Action", remotes.first()->get("Active").toString());
    }
}

JSonNode::SetError VirtualRemote::fastTrigger(FTriggerEvent e, const HexData &data)
{
    if(e==LED){
        sendFastTrigger( e, data);
        return setNumber("LED", data.toInt());
    }

    _eventsHandler[e]->triggerEvent(data.toInt()?"down":"up");
    return setString(eventName(e), data.toInt()?"down":"up");
}

void VirtualRemote::sendFastTrigger(FTriggerEvent e, const HexData &data)
{
    foreach(Remote* r, patchedRemotes())
        r->fastTrigger(e, data);
}


QList<Remote*> VirtualRemote::patchedRemotes() const
{
    QList<Remote*>r;
    if(_group){
        if(_name.startsWith("Global"))
            return SNetworkModel::ptr()->patch()->remoteList(_group->name());
        else{
            QString rName = _group->slave(get("RemoteID").toInt());
            Remote* remote=0;
            if(!rName.isEmpty() && (remote=SNetworkModel::ptr()->remotes()->byName(rName)))
                r.append(remote);
        }
    }else{
        return SNetworkModel::ptr()->patch()->remoteList(_name);
    }
    return r;
}

JSonNode::SetError VirtualRemote::setValue(QString name, QString value)
{
    if(name=="Up" || name=="Down" || name == "Left" || name == "Right" || name=="Action"){
        HexData d((uint8_t)0);
        if(value=="down")
            d=1;
        else if(value!="up")
            return WrongArg;

        return fastTrigger(eventID(name), d);
    }else if(name=="LED"){
        bool success;
        uint8_t intensity = value.toInt(&success);
        if(!success)
            return WrongArg;
        return fastTrigger(LED, intensity);
    }else if(name=="State"){
        foreach(Remote* r, patchedRemotes())
            r->set("State", value);
        return JSonNode::setString(name, value);
    }else if(name=="remoteID" && _group){
        bool success;
        uint8_t id = value.toInt(&success);
        if(!success)
            return WrongArg;
        JSonNode::SetError e = setNumber(name, id);
        updateVirtualRemote();
        return e;
    }

    return JSonNode::setValue(name, value);
}


bool VirtualRemote::execFunction(QString, QStringList, const std::function<void (QString)> &)
{
    return false;
}

void VirtualRemote::generateHelp(bool function)
{
    if(!function){
        addHelp("Up",       "State of the Up button of the virtual remote", false);
        addHelp("Down",     "State of the Down button of the virtual remote", false);
        addHelp("Left",     "State of the Left button of the virtual remote", false);
        addHelp("Right",    "State of the Right button of the virtual remote", false);
        addHelp("Action",   "State of the Action button of the virtual remote", false);
        addHelp("LED",      "Led intensity of the virtual remote", false);
        addHelp("State",    "State of the virtual remote", false);
        if(_group)
            addHelp("remoteID", "ID of the real remote (in the group) this virtual remote is connected", false);

    }
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
            return true;
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
                return true;
            }
        }
        return true;
    }


    return false;
}

void Event::generateHelp(bool function)
{
    if(!function){
        addHelp("any", "Events triggered when an action happened (READONLY)", false);
    }else{
        addHelp("add( event, trigger )", "Add an event definition for a trigger (trigger = \"any\" by default)", true);
        addHelp("remove( event, trigger )", "Remve an event for a trigger (trigger = \"any\" by default)", true);
    }
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


VirtualRemote *VirtualGroup::globalRemote() {return _globalRemote;}

void VirtualGroup::setVirtualRemotesNbr(int nbr)
{
    if(_jsonData.count()<3)
        return;

    int delta = nbr-_jsonData.count()+3;

    while(delta!=0){
        if(delta>0){
            QString rName = "Remote"+QString().setNum(nbr-delta+1);
            QJsonObject remoteJson = VirtualRemote::createVirtualRemoteJSon();
            remoteJson.insert("remoteID", -1);
            createSubNode(rName, remoteJson);
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

void VirtualGroup::addSlave(QString slaveName)
{
    if(_slaves.contains(slaveName))
        return;
    _slaves.append(slaveName);
    setNumber("slavesNbr", _slaves.size());
    updateFromPatch();
}

void VirtualGroup::removeSlave(QString slaveName)
{
    _slaves.removeOne(slaveName);
    setNumber("slavesNbr", _slaves.size());
    updateFromPatch();
}

QString VirtualGroup::slave(int id) const
{
    if(id<_slaves.size() && id>=0)
        return _slaves.at(id);
    return "";
}

QJsonObject VirtualGroup::createGroupJSon()
{
    QJsonObject o;
    o.insert("slavesNbr", 0);
    o.insert("avatarsNbr", 0);
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
            int remoteID = name.mid(6).toInt(&success);
            if(!success){
                delete r;
                return false;
            }
            while(_virtualRemotes.count()<remoteID)
                _virtualRemotes.append(0);
            _virtualRemotes[remoteID-1] = r;
        }
        return true;
    }
    return false;
}

void VirtualGroup::fastTrigger(QString rName, FTriggerEvent e, const HexData &data)
{
    int id = _slaves.indexOf(rName);
    if(id==-1)
        return;

    foreach(VirtualRemote* r, _virtualRemotes){
        if(r->get("remoteID").toInt()==id)
            r->fastTrigger(e, data);
    }

    _globalRemote->fastTrigger(e, data);
}

void VirtualGroup::updateFromPatch()
{
    QStringList updatedSlaves = SNetworkModel::ptr()->patch()->virtualToRemoteMap().value(_name);
    if(_slaves.size() != updatedSlaves.size()){
        _slaves = updatedSlaves;
        setNumber("slavesNbr", _slaves.size());
    }

    _globalRemote->updateVirtualRemote();
    foreach(VirtualRemote* r, _virtualRemotes)
        r->updateVirtualRemote();
}

JSonNode::SetError VirtualGroup::setValue(QString name, QString value)
{
   if(name == "avatarsNbr"){
        bool success = false;
        int nbr = value.toInt(&success);
        if(!success)
            return  WrongArg;
        setVirtualRemotesNbr(nbr);
        return setNumber(name, nbr);
    }

    return JSonNode::setValue(name, value);
}

bool VirtualGroup::execFunction(QString function, QStringList, const std::function<void (QString)> &returnCb)
{
    if(function=="listSlaves"){
        QString r = 0;
        for (int i = 0; i < _slaves.size(); ++i) {
            r+= QString().setNum(i);
            r+= ":\t" + _slaves.at(i) + "\n";
        }
        returnCb(r);
        return true;
    }
    return false;
}

void VirtualGroup::generateHelp(bool function)
{
    if(!function){
        addHelp("slavesNbr", "Number of remote slaves (READONLY)", false);
        addHelp("avatarsNbr", "Number of virtual remotes in the group", false);
        addHelp("GlobalRemote", "A remote to control them all", false);
    }else{
        addHelp("listSlaves()", "List remotes slaves connected to the group", true);
    }
}


