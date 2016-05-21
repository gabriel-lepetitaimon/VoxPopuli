#ifndef VIRTUALNETWORK_H
#define VIRTUALNETWORK_H

#include "eventmodel.h"

class VirtualNetwork: public JSonNode
{
    Q_OBJECT
    EventModel* _eventModel;
public:
    VirtualNetwork(EventModel* eventModel);
    bool createSubNode(QString name, const QJsonObject& data);
};

#endif // VIRTUALNETWORK_H
