#ifndef NETWORKMODEL_H
#define NETWORKMODEL_H

#include "model.h"
#include "singleton.h"

class NetworkModel;
typedef singleton<NetworkModel> SNetworkModel;


class NetworkModel : public JSonModel
{
public:
    NetworkModel();

    void setXbeeUsbPort(std::string port);

protected:
    bool createSubNode(QString name, QJsonValueRef data);
    SetError setValue(QString name, QString value);
};



class RemoteList: public JSonNode
{
public:

    RemoteList(NetworkModel* model);
    bool createSubNode(QString name, QJsonValueRef data);

protected:
};


class Remote: public JSonNode
{
public:

    Remote(QString name, RemoteList* list);

    static QJsonObject generateNode(QString address);


protected:
    SetError setValue(QString name, QString value);
};

#endif // NETWORKMODEL_H
