#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>
#include <RtMidi.h>

#include "eventtrigger.h"
#include "model.h"
#include "singleton.h"
#include "misc.h"

class EventModel;
typedef singleton<EventModel> SEventModel;
class MidiInterface;
class MidiPort;
class VirtualNetwork;

class EventModel : public JSonModel
{
    Q_OBJECT

public:
    EventModel();
    virtual ~EventModel() {}

    bool createSubNode(QString name, const QJsonObject& data);
    MidiInterface*  midi()      {return _midi;}
    VirtualNetwork* virtualNet(){return _virtualNet;}

protected:
    MidiInterface* _midi;
    VirtualNetwork* _virtualNet;
};


class MidiInterface: public JSonNode
{
    Q_OBJECT

    QList<EventTrigger*> _midiEventTriggers;

public:
    MidiInterface(EventModel* eventModel);
    virtual ~MidiInterface() {}
    bool createSubNode(QString name, const QJsonObject& data);

    MidiPort* portByName(bool inPort, QString portName);
    const QList<MidiPort*>& ports() const {return _ports;}

    bool addPort(QString portName, bool inPort, QString port = "");
    bool removePort(QString portName);

    static RtMidiIn*  RtIn()  {return new RtMidiIn(); }
    static RtMidiOut* RtOut() {return new RtMidiOut();}

    void updateEventTriggers();
    void addEventTrigger(EventTrigger* e);
    void removeEventTrigger(EventTrigger* e);

protected:
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    QList<MidiPort*> _ports;
};


class MidiPort: public JSonNode
{
    Q_OBJECT

    RtMidi* _port = 0;
    bool _in;

public:
    MidiPort(QString name, bool inPort, MidiInterface* interface);
    virtual ~MidiPort();
    static QJsonObject createMidiPortJSon(bool inPort, QString port="");

    enum MidiEvent{
        NOTE_OFF            = 0x08,
        NOTE_ON             = 0x09,
        POLY_AFTER_TOUCH    = 0x0A,
        CC                  = 0x0B,
        PROGRAM_CHANGE      = 0x0C,
        AFTER_TOUCH         = 0x0D,
        PITCH_BEND          = 0x0E,
        SYSTEM_COMMON       = 0x0F
    };
    static QString midiEventName(MidiEvent e);
    static bool midiEventFromName(QString name, MidiEvent& e);

    bool isInPort() const {return _in;}
    bool send(const HexData& msg);
    bool send(int8_t channel, MidiEvent eventType, HexData d);

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    static void midiCallback(double timeSptamp, std::vector<unsigned char> *message, void *userData);

    QStringList portsName() const;
    bool openPort(unsigned int portNumber);
};




#endif // EVENTMODEL_H
