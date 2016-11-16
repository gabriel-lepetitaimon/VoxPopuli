#include "eventmodel.h"

#include "virtualnetwork.h"
#include "midiinterface.h"
#include "oscinterface.h"

EventModel::EventModel()
    :JSonModel("eventsProcessor")
{
}

bool EventModel::createSubNode(QString name, const QJsonObject &data)
{
    if(name == "Midi"){
        _midi = new MidiInterface(this);
        if(!_midi->populateNode(data)){
            delete _midi;
            _midi = 0;
            return false;
        }
        return true;
    }
    else if(name == "VirtualNet"){
        _virtualNet = new VirtualNetwork(this);
        if(!_virtualNet->populateNode(data)){
            delete _virtualNet;
            _virtualNet = 0;
            return false;
        }
        return true;
    }else if(name == "OSC"){
        _osc = new OscInterface(this);
        if(!_osc->populateNode(data)){
            delete _osc;
            _osc = 0;
            return false;
        }
        return true;
    }

    return false;
}

MidiInterface *EventModel::midi(){
    return _midi;
}

VirtualNetwork *EventModel::virtualNet(){
    return _virtualNet;
}

OscInterface *EventModel::osc()
{
    return _osc;
}

void EventModel::generateHelp(bool function)
{
    if(!function){
        addHelp("Midi", "Midi interface", false);
        addHelp("OSC", "OSC interface", false);
        addHelp("VirtualNet", "Network of virtual remotes and groups", false);
    }
}
