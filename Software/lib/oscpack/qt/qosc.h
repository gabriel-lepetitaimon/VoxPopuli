#ifndef QOSC_H
#define QOSC_H

#include <QThread>
#include <QString>
#include <QRegExp>
#include <QVariant>

#include "osc/OscPacketListener.h"
#include "osc/OscReceivedElements.h"
#include "ip/UdpSocket.h"
#include "osc/OscOutboundPacketStream.h"


class IpEndpointName;
class UdpListeningReceiveSocket;

namespace qosc {

class Listener: protected QThread, osc::OscPacketListener
{
    Q_OBJECT
    
    QString _addressFilter;
    UdpListeningReceiveSocket* _socket = 0;
    int _port = -1;
    int _state = 0;
    
    QList<QObject*> _listenerObjects;
    
public:
    Listener(QObject *parent = 0);
    virtual ~Listener();
    
    void setAddressFilter(QString addressFilter);
    QString getAddressFilter() const {return _addressFilter;}
    
    bool addListener(QObject* listener);
    bool removeListener(QObject* listener);
    
    bool startServer(int port);
    void stopServer();
	int port() const {return _port;}
signals:
    void msgReceived(QString address, QVariantList param);
    
protected:
    void ProcessMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint);
    void run();
};

QVariantList decodeMsg(const osc::ReceivedMessage &m);
void sendMsg(IpEndpointName endpoint, QString oscAddr, QVariantList args=QVariantList());
bool sendMsg(QString ipAddress, int port, QString oscAddr, QVariantList args=QVariantList());
void sendMsg(unsigned int ipAddress, int port, QString oscAddr, QVariantList args=QVariantList());
void sendMsg(int port, QString oscAddr, QVariantList args=QVariantList());

bool sendToOscStream(osc::OutboundPacketStream& stream, const QVariant &v);

} //namespace osc

#endif // QOSC_H
