#include "virtualnetwork.h"

VirtualNetwork::VirtualNetwork(EventModel *eventModel)
    :JSonNode("VirtualNet", eventModel), _eventModel(eventModel)
{

}

bool VirtualNetwork::createSubNode(QString name, const QJsonObject &data)
{
    return false;
}
