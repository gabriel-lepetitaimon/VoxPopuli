#include "eventmodel.h"

#include "virtualnetwork.h"
#include "misc.h"

#include "networkmodel.h"

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
    }

    return false;
}


/********************************************
 *              Midi Interface              *
 *******************************************/


MidiInterface::MidiInterface(EventModel *eventModel)
    : JSonNode("Midi", eventModel)
{

}

bool MidiInterface::createSubNode(QString name, const QJsonObject &data)
{
    if(data.contains("type")){
        MidiPort* p = new MidiPort(name, data.value("type").toString()=="in", this);
        if(!p->populateNode(data)){
            delete p;
            return false;
        }
        _ports.append(p);
        return true;
    }
    return false;
}

bool MidiInterface::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{

    if(function == "listPort"){
        QString r;
        bool both = true, in = true;
        if(args.size()==1){
            if(!args.first().compare("in", Qt::CaseInsensitive))
                both = false;
            else if(!args.first().compare("out", Qt::CaseInsensitive)){
                in = false;
                both = false;
            }
        }

        if(both || in){
            if(both)
                r+="IN\n";
            RtMidiIn* in = RtIn();
            for(unsigned int i=0; i<in->getPortCount(); i++)
                r+=(both?"\t":"")+QString().setNum(i)+" \""+QString::fromStdString(in->getPortName(i))+"\"\n";
            delete in;
        }
        if(both || !in){
            if(both)
                r+="OUT\n";
            RtMidiOut* out = RtOut();
            for(unsigned int i=0; i<out->getPortCount(); i++)
                r+=(both?"\t":"")+QString().setNum(i)+" \""+QString::fromStdString(out->getPortName(i))+"\"\n";
            delete out;
        }
        r.remove(r.size()-1,1);
        returnCb(r);
        return true;
    }else if(function == "open"){
        if(args.size() != 2 && args.size()!=3){
            returnCb("#Error: Wrong argument number");
            return true;
        }

        //Parsing in or out port
        bool inPort;
        if(!args[1].compare("in",Qt::CaseInsensitive))
            inPort = true;
        else if(!args[1].compare("out",Qt::CaseInsensitive))
            inPort = false;
        else{
            returnCb("#Error: second argument should be \"in\" or \"out\"");
            return true;
        }

        QString midiPort="";
        if(args.size()==3)
            midiPort = args[2];

        if(!addPort(args[0], inPort, midiPort))
            returnCb("#Error: the port name \""+args[0]+"\" is already taken");
        return true;
    }else if(function == "close"){
        if(args.size()!=1){
            returnCb("#Error: Wrong argument number");
            return true;
        }
        if(!removePort(args[0]))
            returnCb("#Error: "+args[0]+" is not a midi port");
        return true;
    }

    return false;
}

MidiPort *MidiInterface::portByName(bool inPort, QString portName)
{
    foreach (MidiPort* p, _ports) {
        if(p->name() == portName && p->isInPort() == inPort)
            return p;
    }
    return 0;
}

bool MidiInterface::addPort(QString portName, bool inPort, QString port)
{
    foreach (MidiPort* p, _ports) {
        if(p->get("port").toString() == port && p->isInPort() == inPort)
            return false;
    }

    createSubNode(portName, MidiPort::createMidiPortJSon(inPort, port));
    updateEventTriggers();
    return true;
}

bool MidiInterface::removePort(QString portName)
{
    for(int i=0; i < _ports.size(); ++i){
        if(_ports[i]->name() == portName){
            removeSubNode(_ports.at(i));
            _ports.removeAt(i);
            updateEventTriggers();
            return true;
        }
    }
    return false;
}

void MidiInterface::updateEventTriggers()
{
    foreach(EventTrigger* e, _midiEventTriggers)
        e->updateEventTrigger("M");
}

void MidiInterface::addEventTrigger(EventTrigger *e)
{
    if(!e || _midiEventTriggers.contains(e))
        return;

    _midiEventTriggers.append(e);
}

void MidiInterface::removeEventTrigger(EventTrigger *e)
{
  if(!e)
      return;
  _midiEventTriggers.removeOne(e);
}


/********************************************
 *               Midi Port                  *
 *******************************************/

MidiPort::MidiPort(QString name, bool inPort, MidiInterface *midiInterface)
    :JSonNode(name, midiInterface, RENAMEABLE), _in(inPort)
{
    if(_in){
        RtMidiIn* in = MidiInterface::RtIn();
        in->setCallback(MidiPort::midiCallback, this);
        _port = in;
    }else
        _port = MidiInterface::RtOut();
}

MidiPort::~MidiPort()
{
    _port->closePort();
}

QJsonObject MidiPort::createMidiPortJSon(bool inPort, QString port)
{
    QJsonObject o;
    o.insert("port", port);
    o.insert("type", inPort?"in":"out");
    return o;
}

QString MidiPort::midiEventName(MidiPort::MidiEvent e)
{
    switch(e){
    case MidiPort::NOTE_OFF:
        return "NoteOff";
    case MidiPort::NOTE_ON:
        return "NoteOn";
    case MidiPort::POLY_AFTER_TOUCH:
        return "PolyAfterTouch";
    case MidiPort::CC:
        return "CC";
    case MidiPort::PROGRAM_CHANGE:
        return "ProgramChange";
    case MidiPort::AFTER_TOUCH:
        return "AfterTouch";
    case MidiPort::PITCH_BEND:
        return "PitchBend";
    case MidiPort::SYSTEM_COMMON:
        return "SystemCommon";
    default:
        return "";
    }
    return "";
}

bool MidiPort::midiEventFromName(QString name, MidiEvent &e)
{
    if(!name.compare("NoteOff", Qt::CaseInsensitive))
        e = MidiPort::NOTE_OFF;
    else if(!name.compare("NoteOn", Qt::CaseInsensitive))
        e = MidiPort::NOTE_ON;
    else if(!name.compare("PolyAfterTouch", Qt::CaseInsensitive))
        e = MidiPort::POLY_AFTER_TOUCH;
    else if(!name.compare("CC", Qt::CaseInsensitive))
        e = MidiPort::CC;
    else if(!name.compare("ProgramChange", Qt::CaseInsensitive))
        e = MidiPort::PROGRAM_CHANGE;
    else if(!name.compare("AfterTouch", Qt::CaseInsensitive))
        e = MidiPort::AFTER_TOUCH;
    else if(!name.compare("PitchBend", Qt::CaseInsensitive))
        e = MidiPort::PITCH_BEND;
    else if(!name.compare("SystemCommon", Qt::CaseInsensitive))
        e = MidiPort::SYSTEM_COMMON;
    else
        return false;
    return true;
}

bool MidiPort::send(const HexData& msg)
{
    if(!_port->isPortOpen())
        return false;
    RtMidiOut* out = dynamic_cast<RtMidiOut*>(_port);
    std::vector<unsigned char>* d = new std::vector<unsigned char>(msg.data());
    out->sendMessage(d);
    delete d;
    return true;
}

bool MidiPort::send(int8_t channel, MidiEvent eventType, HexData d)
{
    int8_t cmin = 0;
    int8_t cmax = 16;
    bool success = true;

    if(channel<=cmax)
        cmin = cmax = channel;

    for(int8_t c =  cmin; c <= cmax; c++){
        uint8_t b1 = 0;
        b1+=c;
        b1+=0x10*eventType;
        d.prepend(b1);
        success &= send(d);
    }

    return success;
}



JSonNode::SetError MidiPort::setValue(QString name, QString value)
{
    if(name == "port"){
        if(value.isEmpty()){
            _port->closePort();
            return setString(name, value);
        }

        bool isNumber;
        int portID = value.toInt(&isNumber);
        if(!isNumber)
            portID = portsName().indexOf(value);

        if(portID != -1){
            if(!openPort(portID))
                return setString(name, "");
        }else
            _port->openVirtualPort(value.toStdString());

        if(isNumber)
            return setNumber(name, portID);
        return setString(name, value);
    }
    return JSonNode::setValue(name, value);
}

bool MidiPort::execFunction(QString function, QStringList args, const std::function<void (QString)> &returnCb)
{
    if(function=="portName"){
        bool isNumber = false;
        int portID = get("port").toInt(&isNumber);
        if(!isNumber)
            returnCb(get("port").toString());
        else
            returnCb(QString::fromStdString(_port->getPortName(portID)));
        return true;

    }else if(function=="send"){
        if(isInPort())
            return false;
        if(args.size()!=1){
            returnCb("#Error: Wrong arguments number");
            return true;
        }
        if(!send(args[0].toStdString()))
            returnCb("#Error: Midi port is closed");
        return true;
    }
    return false;
}

void MidiPort::midiCallback(double , std::vector<unsigned char> *message, void *userData)
{
    MidiPort* port = static_cast<MidiPort*>(userData);
    if((*message)[0]==190){
        Remote* r = SNetworkModel::ptr()->remotes()->byName("R");
        if(!r)
            return;
        r->set("LED", QString().setNum((*message)[2]));
    }

}

QStringList MidiPort::portsName() const
{
    QStringList r;
        for(unsigned int i=0; i<_port->getPortCount(); i++)
            r.append(QString::fromStdString(_port->getPortName(i)));
        return r;
}

bool MidiPort::openPort(unsigned int portNumber)
{
    if(_port->getPortCount()<=portNumber)
        return false;

    _port->openPort(portNumber, "VoxPopuli");
    return true;
}
