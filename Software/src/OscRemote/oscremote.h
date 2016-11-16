#ifndef OSCREMOTE_H
#define OSCREMOTE_H

#include <QObject>
#include "qosc.h"

class OSCRemote: public QObject
{
    Q_OBJECT
    
    qosc::Listener* _listener;
    QString _ip = "localhost";
    int _outPort = 9001;
    int _inPort = 9002;
    QString _oscAddress = "/VPDeamon/R1";
    QObject* _qml;
public:
    OSCRemote(QObject *qml);
    
public slots:
    void readOscMsg(QString addr, QVariantList args);
    void buttonChanged(QString button, bool state);
    
protected:
    void ledChanged(int ledID, float value);
};

#endif // OSCREMOTE_H
