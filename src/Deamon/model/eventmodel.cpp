#include "eventmodel.h"

#include "virtualnetwork.h"
#include "misc.h"

#include "networkmodel.h"

EventModel::EventModel()
    :JSonModel("eventsProcessor")
{
    initModel();
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
    return true;
}

bool MidiInterface::removePort(QString portName)
{
    for(int i=0; i < _ports.size(); ++i){
        if(_ports[i]->name() == portName){
            removeSubNode(_ports.at(i));
            _ports.removeAt(i);
            return true;
        }
    }
    return false;
}


/********************************************
 *               Midi Port                  *
 *******************************************/

MidiPort::MidiPort(QString name, bool inPort, MidiInterface *interface)
    :JSonNode(name, interface, RENAMEABLE), _in(inPort)
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

bool MidiPort::send(QString msg)
{
    std::vector<unsigned char> hex = hexStrToInt(msg.toStdString());
    return send(hex);
}

bool MidiPort::send(std::vector<unsigned char> msg)
{
    if(!_port->isPortOpen())
        return false;
    RtMidiOut* out = dynamic_cast<RtMidiOut*>(_port);
    out->sendMessage(&msg);
    return true;
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
        if(!send(args[0]))
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
