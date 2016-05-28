#ifndef PATCH_H
#define PATCH_H

#include "model.h"

class NetworkModel;
class PatchLink;
class Remote;
class VirtualRemote;
class VirtualGroup;

class Patch: public JSonNode{
    NetworkModel* _model;
    QList<PatchLink*> _links;

    QMap<QString, QStringList> _remoteToVirtual;
    QMap<QString, QStringList> _virtualToRemote;

public :
    Patch(NetworkModel* model);
    virtual ~Patch() {}

    inline bool addLink(Remote* remote, VirtualRemote* vRemote);
    inline bool addLink(Remote* remote, VirtualGroup* vGroup);
    inline bool removeLink(Remote* remote, VirtualRemote* vRemote);
    inline bool removeLink(Remote* remote, VirtualGroup* vGroup);
    inline bool removeLinks(Remote* remote);
    inline bool removeLinks(VirtualRemote* vRemote);
    inline bool removeLinks(VirtualGroup* vGroups);

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    SetError parseArray(QString name, QStringList value);

    bool addLink(QString rName, QString vName);
    bool addEntry(QString rName, QString vName);
    bool removeLink(QString rName, QString vName);
    bool removeRemoteLinks(QString rName);
    bool removeVirtualLinks(QString vName);

};
#endif // PATCH_H
