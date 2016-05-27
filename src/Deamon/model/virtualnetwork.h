#ifndef VIRTUALNETWORK_H
#define VIRTUALNETWORK_H

#include <QList>
#include <QMap>
#include "eventmodel.h"
//#include "eventtrigger.h"

//class VirtualRemote;
//class Event;
//class Group;

class VirtualNetwork: public JSonNode
{
    Q_OBJECT
//    EventModel* _eventModel;
public:
    VirtualNetwork(EventModel* eventModel);
    virtual ~VirtualNetwork() {}
//    bool createSubNode(QString name, const QJsonObject& data);

//    bool addRemote(QString name="");
//    VirtualRemote* remoteByName(QString name);
//    bool removeRemote(QString name);

//    bool addGroup(QString name=0);
//    Group* groupByName(QString name);
//    bool removeGroup(QString name);

//    void autoGenerateRemote();

//    EventModel* eventModel() {return _eventModel;}

//protected:
//    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});


//private:
//    QList<VirtualRemote*> _vRemotes;
//    QList<Group*> _vGroups;
};

//class VirtualRemote: public JSonNode
//{
//    Q_OBJECT

//    VirtualNetwork* _vNet;

//public:
//    VirtualRemote(QString name, VirtualNetwork* virtualNet);
//    virtual ~VirtualRemote();
//    bool createSubNode(QString name, const QJsonObject& data);

//    VirtualNetwork* virtualNet() {return _vNet;}
//    static QJsonObject createVirtualRemoteJSon();
//protected:
//    SetError setValue(QString name, QString value);
//    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

//    QList<Event*> _eventsHandler;
//};

//class Event: public JSonNode
//{
//    Q_OBJECT
//    VirtualRemote* _remote;
//    bool _inEvents;
//    EventTrigger* _anyTrigger;
//    QMap<QString, EventTrigger*> _triggers;

//public:
//    Event(VirtualRemote* remote, bool inEvents);

//    void addEvent(QString description, QString trigger="any");
//    bool removeEvent(QString definition, QString trigger="any");
//    EventTrigger* eventByTrigger(QString trigger="any");
//    QStringList triggers() const;

//    bool triggerEvent(QString trigger);

//    bool isInEvents() const;
//    VirtualRemote* remote();

//    static QJsonObject createEventJSon();

//protected:
//    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
//};



//class Group: public JSonNode
//{
//    Q_OBJECT
//    VirtualNetwork* _vNet;
//public:
//    Group(QString name, VirtualNetwork* virtualNet);
//    virtual ~Group() {}

//    VirtualNetwork* virtualNet();

//    void setSlavesNbr(int nbr);

//    VirtualRemote* globalRemote();
//    void setSpecialRemotesNbr(int nbr);
//    int getSpecialRemoteNbr() const;
//    VirtualRemote* specialRemote(int id);

//    static QJsonObject createGroupJSon();

//protected:
//    VirtualRemote* _globalRemote;
//    QList<VirtualRemote*> _specialRemotes;
//    SetError setValue(QString name, QString value);
//    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});


//};

#endif // VIRTUALNETWORK_H
