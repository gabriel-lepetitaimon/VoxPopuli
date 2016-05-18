#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H

#include <QList>

#include "model.h"
#include "singleton.h"
#include "xbee/xbeeremote.h"

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

    bool createSubNode(QString name, const QJsonObject& data);

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    RemoteList* _remotes;
};



class RemoteList: public JSonNode
{
    Q_OBJECT

public:
    RemoteList(NetworkModel* model);
    virtual ~RemoteList(){}
    bool createSubNode(QString name, const QJsonObject& data);

    //    Remote* remoteByName(QString name);
    //    Remote* remoteByAddr(QString addr);

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

    enum Button{
        TOP,
        BOTTOM,
        RIGHT,
        LEFT,
        CENTER
    };

    Remote(QString name, RemoteList* list);
    virtual ~Remote(){}

    static QJsonObject createRemoteJSon(QString address);
    void setButtonState(Button b, bool pressed);


    XBeeRemote* remote();

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

};

#endif // NETWORKMODEL_H
