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
class RealRemote;
class EmulatedRemote;

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
    virtual void generateHelp(bool function);

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
    RealRemote* byMAC(QString mac);
    EmulatedRemote* byOSC(QString osc);

    QList<Remote*>& remotes() {return _remotes;}

    QStringList remotesNames() const;

public slots:
    bool addRemote(QString address);
    void removeRemote(QString address);
    bool emulateRemote(QString name, QString osc="", QString ipPort="");
    bool removeEmulatedRemote(QString name);

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual void generateHelp(bool function);

    QList<Remote*> _remotes;
};


class Remote: public JSonNode
{

public:
    virtual ~Remote();

    void setButtonState(XBEE_MSG_TYPE b, bool pressed);
    virtual SetError fastTrigger(FTriggerEvent e, const HexData& v);

protected:
    Remote(QString name, RemoteList* list);
    static QJsonObject createRemoteJSon();

    virtual SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual void generateHelp(bool function);
};

class RealRemote: public Remote{

    XBeeRemote* _remote=0;

public:
    RealRemote(QString name, QString mac, RemoteList* list);
    static QJsonObject createRealRemoteJSon(QString address);

    virtual ~RealRemote();

    QString macAddress() const {return _jsonData.value("MAC").toString("");}
    XBeeRemote* remote();
    void setSignalStrength(int dB);

    virtual SetError fastTrigger(FTriggerEvent e, const HexData& v);

protected:
    virtual SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual void generateHelp(bool function);
};

class EmulatedRemote: public Remote{

public:
    EmulatedRemote(QString name, RemoteList* list, QString oscAddress = "", QString ipPort = "");
    static QJsonObject createEmulatedRemoteJSon(QString oscAddress = "", QString ipPort="");

    virtual ~EmulatedRemote();

    QString oscAddress() const {return _jsonData.value("osc").toString("");}
    QString forwardIP() const;
    int forwardPort() const;

    void readOsc(QVariantList args);
    virtual SetError fastTrigger(FTriggerEvent e, const HexData& v);

protected:
    virtual SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});
    virtual void generateHelp(bool function);
};



#endif // NETWORKMODEL_H
