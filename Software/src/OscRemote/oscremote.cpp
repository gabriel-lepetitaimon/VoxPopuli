#include "oscremote.h"

#include <QDebug>

OSCRemote::OSCRemote(QObject *qml)
    :_qml(qml)
{
    _listener = new qosc::Listener(this);
    _listener->addListener(this);
    _listener->startServer(_inPort);
    QVariantList args;
    args<<"emulate"<<_ip+':'+QString().setNum(_inPort);
    qosc::sendMsg(_ip, _outPort, _oscAddress, args);
    
    QObject::connect(_qml, SIGNAL(buttonChanged(QString,bool)), this, SLOT(buttonChanged(QString,bool)));
}

void OSCRemote::readOscMsg(QString addr, QVariantList args)
{
    qWarning()<<"OSC|"<<addr<<args;
    if(!addr.startsWith(_oscAddress))
        return;
    qWarning()<<"1";
    if(args.size()!=2)
        return;
        qWarning()<<"2";
    bool ok = true;
    float intensity = (float)(args.at(1).toInt(&ok))/255.0f;
    if(!ok)
        return;
        qWarning()<<"3";
    
    QString led = args.first().toString();
    if(led == "LED1")
        ledChanged(1, intensity);
    else if(led == "LED2")
        ledChanged(2, intensity);
    else if(led == "LED3")
        ledChanged(3, intensity);
    else if(led == "LED"){
        ledChanged(1, intensity);
        ledChanged(2, intensity);
        ledChanged(3, intensity);
    }
    
}

void OSCRemote::buttonChanged(QString button, bool state)
{
    QVariantList args;
    args<<button<<state;
    qosc::sendMsg(_ip, _outPort, _oscAddress, args);
}

void OSCRemote::ledChanged(int ledID, float value)
{
    QMetaObject::invokeMethod(_qml, "ledChanged",
            Q_ARG(QVariant, ledID),
            Q_ARG(QVariant, value));
}
