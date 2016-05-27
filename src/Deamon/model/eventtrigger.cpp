#include <string>

#include "eventtrigger.h"
#include "eventmodel.h"
#include "misc.h"


EventTrigger::EventTrigger(QString name, bool inEvent, QObject *parent)
    :QObject(parent), _name(name), _inEvent(inEvent)
{
}

QStringList EventTrigger::definitions() const
{
    return _callbacks.keys();
}

void EventTrigger::addEvent(QString definition)
{

    QStringList def = definition.split('|');
    if(def.isEmpty())
        return;

    QList<MidiPort*> ports = SEventModel::ptr()->midi()->ports();
    foreach(MidiPort* port, ports){
        if(port->isInPort()!=_inEvent)
            continue;
        if(port->name() != def[0])
            continue;

        if(_inEvent){

        }else{

        }
    }
}

void EventTrigger::addEvent(QString definition, EventCb cb)
{
    mutex.lock();
    _callbacks.insert(definition, cb);
    mutex.unlock();
}

void EventTrigger::addEvent(QString definition, SimplifiedEventCb cb)
{
    addEvent(definition,  [cb](uint8_t){cb();});
}

bool EventTrigger::removeEvent(QString definition)
{
    mutex.lock();
    int r = _callbacks.remove(definition);
    mutex.unlock();
    return r;
}

void EventTrigger::trigger(uint8_t data)
{
    mutex.lock();
    foreach(EventCb cb, _callbacks.values())
        cb(data);
    mutex.unlock();
    emit triggrered(data);
}
