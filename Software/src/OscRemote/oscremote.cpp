#include "oscremote.h"

#include <QDebug>

OSCRemote::OSCRemote(QObject *qml)
    :_qml(qml)
{
    _listener = new qosc::Listener(this);
    _listener->addListener(this);
    _listener->startServer(_inPort);
    QVariantList args;
    args<<"emulate";
    qosc::sendMsg(_ip, _outPort, _oscAddress, args);
    
    QObject::connect(_qml, SIGNAL(buttonChanged(QString,bool)), this, SLOT(buttonChanged(QString,bool)));
}

void OSCRemote::readOscMsg(QString addr, QVariantList args)
{
    if(!addr.startsWith(_oscAddress))
        return;
    if(args.size()!=1)
        return;
    bool ok = true;
    float intensity = (float)(args.first().toInt(&ok))/255.0f;
    if(!ok)
        return;
    
    addr = addr.remove(0,_oscAddress.length()+1);
    if(addr == "LED1")
        ledChanged(1, intensity);
    else if(addr == "LED2")
        ledChanged(2, intensity);
    else if(addr == "LED3")
        ledChanged(3, intensity);
    else if(addr == "LED"){
        ledChanged(1, intensity);
        ledChanged(2, intensity);
        ledChanged(3, intensity);
    }
    
}

void OSCRemote::buttonChanged(QString button, bool state)
{
    QVariantList args;
    args<<state;
    qosc::sendMsg(_ip, _outPort, _oscAddress+"/"+button, args);
    qWarning()<<_oscAddress+"/"+button<<args;
}

void OSCRemote::ledChanged(int ledID, float value)
{
    QMetaObject::invokeMethod(_qml, "ledChanged",
            Q_ARG(QVariant, ledID),
            Q_ARG(QVariant, value));
}
