#include <QJsonArray>

#include "patch.h"

#include "networkmodel.h"
#include "virtualnetwork.h"

/********************************************
 *              Patch                      *
 *******************************************/

Patch::Patch(NetworkModel *model)
    : JSonNode("Patch", model), _model(model)
{

}

bool Patch::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function=="connect"){
        if(args.size()<2){
            returnCb("#Error: Wrong arguments number, expected remote and virtual remote names");
            return true;
        }
        addLink(args[0], args[1]);
        return true;
    }else if(function =="disconnect"){
        if(args.size()<2){
            returnCb("#Error: Wrong arguments number, expected remote and virtual remote names");
            return true;
        }
        removeLink(args[0], args[1]);
        return true;
    }else if(function == "disconnectRemote"){
        if(args.size()<1){
            returnCb("#Error: Wrong arguments number, expected remote name");
            return true;
        }
        removeRemoteLinks(args[0]);
        return true;
    }else if(function == "disconnectVirtual" || function == "disconnectVirtualRemote" || function == "disconnectVirtualGroup"){
        if(args.size()<1){
            returnCb("#Error: Wrong arguments number, expected remote name");
            return true;
        }
        removeVirtualLinks(args[0]);
        return true;
    }
    return false;
}

JSonNode::SetError Patch::parseArray(QString name, QStringList value)
{
    foreach(QString vName, value){
        if(!addEntry(name, vName))
            return WrongArg;
    }
    return NoError;
}


//  -----------  LINK HANDLER  -------------

bool Patch::addEntry(QString rName, QString vName)
{
    auto rEntry = _remoteToVirtual.find(rName);
    if(rEntry == _remoteToVirtual.end())
        _remoteToVirtual.insert(rName, QStringList(vName));
    else if(!rEntry.value().contains(vName))
        rEntry.value().append(vName);
    else
        return false;

    auto vEntry = _virtualToRemote.find(vName);
    if(vEntry == _virtualToRemote.end())
        _virtualToRemote.insert(vName, QStringList(rName));
    else if(!vEntry.value().contains(rName))
        vEntry.value().append(rName);
    else
        return false;

    return true;
}

bool Patch::removeLink(QString rName, QString vName)
{
    auto rEntry = _remoteToVirtual.find(rName);
    if(rEntry==_remoteToVirtual.end())
        return false;
    if(rEntry->removeOne(vName)){
        QJsonArray a = _jsonData[rName].toArray();
        for(auto it = a.begin(); it!=a.end(); it++){
            if((*it).toString()==vName){
                a.erase(it);
                break;
            }
        }
        if(!a.isEmpty())
            _jsonData[rName] = a;
        else
            _jsonData.remove(rName);
        valueChanged(rName);
    }else
        return false;

    if(rEntry->isEmpty())
        _remoteToVirtual.erase(rEntry);

    auto vEntry = _virtualToRemote.find(vName);
    if(vEntry != _virtualToRemote.end())
        vEntry->removeOne(rName);
    else
        return false;

    if(vEntry->isEmpty())
        _virtualToRemote.erase(vEntry);

    return true;
}

bool Patch::addLink(QString rName, QString vName)
{
    if(!addEntry(rName, vName))
        return false;

    QJsonArray a;
    auto link = _jsonData.find(rName);
    if(link!=_jsonData.end())
        a = (*link).toArray();
    a.push_back(vName);
    _jsonData[rName]=a;
    valueChanged(rName);

    return true;
}

bool Patch::removeRemoteLinks(QString rName)
{
    auto rEntry = _remoteToVirtual.find(rName);
    if(rEntry == _remoteToVirtual.end())
        return false;

    foreach(QString vName, rEntry.value()){
        auto vEntry = _virtualToRemote.find(vName);
        if(vEntry != _virtualToRemote.end()){
            vEntry->removeOne(rName);
            if(vEntry->isEmpty())
                _virtualToRemote.erase(vEntry);
        }
    }

    _jsonData.remove(rName);
    valueChanged(rName);
    _remoteToVirtual.erase(rEntry);

    return true;
}

bool Patch::removeVirtualLinks(QString vName)
{
    auto vEntry = _virtualToRemote.find(vName);
    if(vEntry == _virtualToRemote.end())
        return false;

    foreach(QString rName, vEntry.value()){
        auto rEntry = _remoteToVirtual.find(rName);
        if(rEntry==_remoteToVirtual.end())
            continue;

        if(rEntry->removeOne(vName)){
            if(rEntry->isEmpty())
                _remoteToVirtual.erase(rEntry);

            QJsonArray a = _jsonData[rName].toArray();
            for(auto it = a.begin(); it!=a.end(); it++){
                if((*it).toString()==vName){
                    a.erase(it);
                    break;
                }
            }
            if(!a.isEmpty())
                _jsonData[rName] = a;
            else
                _jsonData.remove(rName);
            valueChanged(rName);
        }
    }

    _virtualToRemote.erase(vEntry);

    return true;
}


bool Patch::addLink(Remote *remote, VirtualRemote *vRemote)
{
    return addLink(remote->name(), vRemote->name());
}

bool Patch::addLink(Remote *remote, VirtualGroup *vGroup)
{
    return addLink(remote->name(), vGroup->name());
}


bool Patch::removeLink(Remote *remote, VirtualRemote *vRemote)
{
    return removeLink(remote->name(), vRemote->name());
}

bool Patch::removeLink(Remote *remote, VirtualGroup *vGroup)
{
    return removeLink(remote->name(), vGroup->name());
}

bool Patch::removeLinks(Remote *remote)
{
    return removeRemoteLinks(remote->name());
}

bool Patch::removeLinks(VirtualRemote *vRemote)
{
    return removeVirtualLinks(vRemote->name());
}

bool Patch::removeLinks(VirtualGroup *vGroups)
{
    return removeVirtualLinks(vGroups->name());
}
