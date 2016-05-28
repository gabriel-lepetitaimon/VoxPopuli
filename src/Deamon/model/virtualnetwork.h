#ifndef VIRTUALNETWORK_H
#define VIRTUALNETWORK_H

#include <QList>
#include <QMap>
#include "eventmodel.h"
#include "eventtrigger.h"

class VirtualRemote;
class Event;
class VirtualGroup;

class VirtualNetwork: public JSonNode
{
    Q_OBJECT
    EventModel* _eventModel;
public:
    VirtualNetwork(EventModel* eventModel);
    virtual ~VirtualNetwork() {}
    bool createSubNode(QString name, const QJsonObject& data);

    bool addRemote(QString name="");
    VirtualRemote* remoteByName(QString name);
    bool removeRemote(QString name);

    bool addGroup(QString name=0);
    VirtualGroup* groupByName(QString name);
    bool removeGroup(QString name);

    void autoGenerateRemote();

    EventModel* eventModel() {return _eventModel;}

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});


private:
    QList<VirtualRemote*> _vRemotes;
    QList<VirtualGroup*> _vGroups;
};

class VirtualRemote: public JSonNode
{
    Q_OBJECT

    VirtualNetwork* _vNet;
    VirtualGroup* _group=0;

public:
    VirtualRemote(QString name, VirtualNetwork* virtualNet);
    VirtualRemote(QString name, VirtualGroup* group);

    virtual ~VirtualRemote() {}
    bool createSubNode(QString name, const QJsonObject& data);

    VirtualNetwork* virtualNet() {return _vNet;}
    static QJsonObject createVirtualRemoteJSon();
protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    QList<Event*> _eventsHandler;

private:
    void initEventsHandler();
};

class Event: public JSonNode
{
    Q_OBJECT
    VirtualRemote* _remote;
    bool _inEvents;
    EventTrigger* _anyTrigger;
    QMap<QString, EventTrigger*> _triggers;

public:
    Event(QString name, VirtualRemote* remote, bool inEvents);

    bool addEvent(QString description, QString trigger="any");
    bool removeEvent(QString definition, QString trigger="any");
    EventTrigger* eventByTrigger(QString trigger="any");
    QStringList triggers() const;

    bool triggerEvent(QString trigger);

    bool isInEvents() const;
    VirtualRemote* remote();

    static QJsonObject createEventJSon();

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual JSonNode::SetError parseArray(QString name, QStringList value);
};



class VirtualGroup: public JSonNode
{
    Q_OBJECT
    VirtualNetwork* _vNet;
public:
    VirtualGroup(QString name, VirtualNetwork* virtualNet);
    virtual ~VirtualGroup() {}

    VirtualNetwork* virtualNet();

    void setSlavesNbr(int nbr);

    VirtualRemote* globalRemote();
    void setVirtualRemotesNbr(int nbr);
    int getVirtualRemoteNbr() const {return _virtualRemotes.size();}
    VirtualRemote* virtualRemote(int id);

    static QJsonObject createGroupJSon();
    bool createSubNode(QString name, const QJsonObject& data);

protected:
    VirtualRemote* _globalRemote;
    QList<VirtualRemote*> _virtualRemotes;
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});


};

#endif // VIRTUALNETWORK_H
