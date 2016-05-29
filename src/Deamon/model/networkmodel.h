#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H

#include <QList>

#include "model.h"
#include "singleton.h"
#include "xbee/xbeeremote.h"
#include "patch.h"

class NetworkModel;
typedef singleton<NetworkModel> SNetworkModel;
class RemoteList;
class Remote;

class NetworkModel : public JSonModel
{
public:
    NetworkModel();
    virtual ~NetworkModel(){}

    void setXbeeUsbPort(std::string port);
    RemoteList*     remotes()   {return _remotes;}
    Patch*          patch()     {return _patch;}

    bool createSubNode(QString name, const QJsonObject& data);

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    RemoteList* _remotes;
    Patch* _patch;
};



class RemoteList: public JSonNode
{
    Q_OBJECT

public:
    RemoteList(NetworkModel* model);
    virtual ~RemoteList(){}
    bool createSubNode(QString name, const QJsonObject& data);

    Remote* byName(QString name);
    Remote* byAddr(QString addr);

    QList<Remote*>& remotes() {return _remotes;}

public slots:
    bool addRemote(QString address);
    void removeRemote(QString address);

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    QList<Remote*> _remotes;
};


class Remote: public JSonNode
{

    XBeeRemote* _remote=0;

public:

    Remote(QString name, QString mac, RemoteList* list);
    virtual ~Remote();

    static QJsonObject createRemoteJSon(QString address);
    void setButtonState(XBEE_MSG_TYPE b, bool pressed);
    void setSignalStrength(int dB);

    QString macAddress() const {return _jsonData.value("MAC").toString("");}
    XBeeRemote* remote();

    SetError fastTrigger(FTriggerEvent e, const HexData& v);

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

};




#endif // NETWORKMODEL_H
