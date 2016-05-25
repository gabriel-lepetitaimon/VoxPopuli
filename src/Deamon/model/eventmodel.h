#ifndef EVENTMODEL_H
#define EVENTMODEL_H

#include <QObject>
#include <RtMidi.h>

#include "model.h"
#include "singleton.h"

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
public:
    MidiInterface(EventModel* eventModel);
    virtual ~MidiInterface() {}
    bool createSubNode(QString name, const QJsonObject& data);

    MidiPort* portByName(bool inPort, QString portName);

    bool addPort(QString portName, bool inPort, QString port = "");
    bool removePort(QString portName);

    static RtMidiIn*  RtIn()  {return new RtMidiIn(); }
    static RtMidiOut* RtOut() {return new RtMidiOut();}

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

    bool isInPort() const {return _in;}
    bool send(QString msg);
    bool send(std::vector<unsigned char> msg);

protected:
    SetError setValue(QString name, QString value);
    virtual bool execFunction(QString function, QStringList args, const std::function<void(QString)>& returnCb=[](QString){});

    static void midiCallback(double timeSptamp, std::vector<unsigned char> *message, void *userData);

    QStringList portsName() const;
    bool openPort(unsigned int portNumber);
};




#endif // EVENTMODEL_H
