#include <string>

#include "eventtrigger.h"
#include "eventmodel.h"
#include "midiinterface.h"
#include "misc.h"

#ifdef X11_SUPPORT
#include "X11/Xlib.h"
#include "X11/extensions/XTest.h"
#endif

EventTrigger::EventTrigger(QString name, bool inEvent, QObject *parent)
    :QObject(parent), _name(name), _inEvent(inEvent)
{
}

EventTrigger::~EventTrigger()
{
    foreach (EventCb* cb, _callbacks.values()){
        if(cb)
            delete cb;
    }
    if(SEventModel::ptr()->midi())
        SEventModel::ptr()->midi()->removeEventTrigger(this);
}

QStringList EventTrigger::definitions() const
{
    return _callbacks.keys();
}

bool EventTrigger::addEvent(QString definition)
{
    QMap<QString, EventCb*>::iterator it;
    mutex.lock();
    it = _callbacks.insert(definition, 0);
    mutex.unlock();
    if(!updateEventTrigger(it)){
        mutex.lock();
        _callbacks.erase(it);
        mutex.unlock();
        return false;
    }
    if(definition.startsWith('M'))
        SEventModel::ptr()->midi()->addEventTrigger(this);
    return true;
}

void EventTrigger::addEvent(QString definition, EventCb cb)
{
    mutex.lock();
    _callbacks.insert(definition, new EventCb(cb));
    mutex.unlock();

    if(definition.startsWith('M'))
        SEventModel::ptr()->midi()->addEventTrigger(this);
}

void EventTrigger::addEvent(QString definition, SimplifiedEventCb cb)
{
    addEvent(definition,  [cb](uint8_t){cb();});
}

bool EventTrigger::removeEvent(QString definition)
{
    auto r = _callbacks.find(definition);
    if(r == _callbacks.end())
        return false;

    mutex.lock();
    if(r.value())
        delete r.value();
    _callbacks.erase(r);
    mutex.unlock();

    if(definition.startsWith('M')){
        foreach(QString d, _callbacks.keys()){
            if(d.startsWith('M'))
                return true;
        }
        SEventModel::ptr()->midi()->removeEventTrigger(this);
    }

    return true;
}

void EventTrigger::updateEventTrigger(QString eventType)
{
    for(auto it=_callbacks.begin(); it!=_callbacks.end(); it++){
        if(eventType.isEmpty() || it.key().startsWith(eventType))
            updateEventTrigger(it);
    }
}

bool EventTrigger::updateEventTrigger(QMap<QString, EventCb*>::iterator it)
{
    QStringList def = it.key().split('|');
    if(def.isEmpty())
        return false;

    if(def[0]=="M"){
        // -----  MIDI EVENT  -----
        if(def.size()!=4)
            return false;

        bool success;
        int8_t channel = def[1].mid(def[1].indexOf(':')+1).toInt(&success);
        if(!success){
            if(def[1].mid(def[1].indexOf(':')) == "*")
                channel = 17;
            else
                return false;
        }

        HexData d = HexData::fromHexStr(def[3].toStdString());
        if(d.size()<1 || d.size()>2)
            return false;

        MidiPort::MidiEvent event = (MidiPort::MidiEvent)def[2].toInt(&success);
        if(!success && !MidiPort::midiEventFromName(def[2],event))
            return false;
        if(!SEventModel::ptr())
            return true;
        MidiPort* port = SEventModel::ptr()->midi()->portByName(_inEvent, def[1].left(def[1].indexOf(':')));
        if(!port)
            return true;

        if(_inEvent){
            return true;
        }else{
            if(it.value())
                delete it.value();
            it.value() = new EventCb([channel,event, d, port](uint8_t data){port->send(channel, event, d.size()==1?d+data:d);});
        }
        return true;
    }else if(def[0]=="K"){
        // -----  KEYBOARD  EVENT  -----
        if(_inEvent)
            return false;
        if(def.size()!=3)
            return false;
        if(it.value())
            return true;
#ifdef X11_SUPPORT
        Display* xdp = XOpenDisplay(NULL);
        if(!xdp)
            return true;
        bool on = false;
        if(!def[2].compare("down", Qt::CaseInsensitive))
            on = true;
        else if(def[2].compare("up", Qt::CaseInsensitive))
            return false;

        KeySym keySym = XStringToKeysym(def[1].toStdString().data());
        if(!keySym)
            return false;
        KeyCode key = XKeysymToKeycode(xdp, keySym);
        if(!key)
            return false;
        it.value() = new EventCb([xdp, key, on](uint8_t){
            XTestGrabControl (xdp, True);
            XTestFakeKeyEvent(xdp, key, on, 0);
            XSync (xdp, False);
            XTestGrabControl (xdp, False);
        });
#else
        qWarning()<<"Keyboard events are only supported via X11 on unix...";
#endif
        return true;
    }


    return false;
}

void EventTrigger::trigger(uint8_t data)
{
    mutex.lock();
    foreach(EventCb* cb, _callbacks.values())
        if(cb) (*cb)(data);
    mutex.unlock();
    emit triggrered(data);
}
