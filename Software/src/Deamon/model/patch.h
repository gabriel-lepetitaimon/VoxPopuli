#ifndef PATCH_H
#define PATCH_H

#include <QMutex>
#include <QList>
#include <QString>

#include "model.h"
#include "misc.h"

class NetworkModel;
class PatchLink;
class Remote;
class VirtualRemote;
class VirtualGroup;


enum FTriggerEvent{
    UP=0,
    DOWN=1,
    RIGHT=2,
    LEFT=3,
    ACTION=4,
    LED=5,
    LED1=6,
    LED2=7,
    LED3=8
};

QString eventName(FTriggerEvent e);
FTriggerEvent eventID(QString e);


class Patch: public JSonNode{
    NetworkModel* _model;
    QList<PatchLink*> _links;

    QMap<QString, QStringList> _remoteToVirtual;
    QMap<QString, QStringList> _virtualToRemote;

    QMutex mutex;

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

    void fastTriggerRemote(QString rName, FTriggerEvent e, const HexData& v);

    QList<Remote*> remoteList(QString virtualRemote);
    const QMap<QString, QStringList> & remoteToVirtualMap() const {return _remoteToVirtual;}
    const QMap<QString, QStringList> & virtualToRemoteMap() const {return _virtualToRemote;}
protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    SetError parseArray(QString name, QStringList value);
    virtual void generateHelp(bool function);


    bool addLink(QString rName, QString vName);
    bool addEntry(QString rName, QString vName);
    bool removeLink(QString rName, QString vName);
    bool removeRemoteLinks(QString rName);
    bool removeVirtualLinks(QString vName);

};
#endif // PATCH_H
