#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>



#include "model.h"
#include "singleton.h"
#include "misc.h"

class EventModel;
typedef singleton<EventModel> SEventModel;
class MidiInterface;
class MidiPort;
class VirtualNetwork;
class OscInterface;

class EventModel : public JSonModel
{
    Q_OBJECT

public:
    EventModel();
    virtual ~EventModel() {}

    bool createSubNode(QString name, const QJsonObject& data);
    MidiInterface*  midi();
    VirtualNetwork* virtualNet();
    OscInterface* osc();

protected:
    MidiInterface* _midi;
    VirtualNetwork* _virtualNet;
    OscInterface* _osc;

    virtual void generateHelp(bool function);
};





#endif // EVENTMODEL_H
