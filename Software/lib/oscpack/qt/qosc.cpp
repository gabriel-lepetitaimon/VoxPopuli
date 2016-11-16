#include "qosc.h"

#include <QDebug>

#define OUTPUT_BUFFER_SIZE 1024
namespace qosc{

/********************************************
 *              OSC Listener                *
 *******************************************/
Listener::Listener(QObject *parent)
    : QThread(parent), osc::OscPacketListener()
{ 
}
Listener::~Listener()
{
    stopServer();
    delete _socket;
}

void Listener::setAddressFilter(QString addressFilter){
    _addressFilter = QRegExp(addressFilter);
}

void Listener::setAddressStartFilter(QString addressStart)
{
    _addressFilter = QRegExp("^" + addressStart);
}

bool Listener::addListener(QObject *listener)
{
    if(_listenerObjects.contains(listener))
        return false;
    
    _listenerObjects.append(listener);
    return true;
}

bool Listener::removeListener(QObject *listener)
{
    return _listenerObjects.removeOne(listener);
}

bool Listener::startServer(int port)
{
    if(_port != -1)
        stopServer();
    
    _port = port;

    try{
		_socket = new UdpListeningReceiveSocket( IpEndpointName( IpEndpointName::ANY_ADDRESS, _port ), this );
	}catch(std::runtime_error &e){
		qWarning()<<"Unable to connect to port: "<<_port<<"...";
		qWarning()<<"   std::runtime_error: "<<e.what();
		return false;
	}
    
    start();
    
    return true;
}

void Listener::stopServer()
{
    _socket->AsynchronousBreak();
    _port  = -1;
}

void Listener::ProcessMessage(const osc::ReceivedMessage &m, const IpEndpointName &/*remoteEndpoint*/)
{
    QString addr = m.AddressPattern();
    if(!_addressFilter.exactMatch(addr))
        return;
    
    QVariantList l = decodeMsg(m);
    foreach(QObject *o, _listenerObjects)
        QMetaObject::invokeMethod(o, "readOscMsg", Qt::QueuedConnection,
                                  Q_ARG(QString, addr),
                                  Q_ARG(QVariantList, l));
}

void Listener::run()
{   
	_socket->RunUntilSigInt();
	return;
}

/******************************************
 *            Decode Message              *
 ******************************************/

QVariantList decodeMsg(const osc::ReceivedMessage &m)
{
    QVariantList l;
    for(auto it = m.ArgumentsBegin(); it!=m.ArgumentsEnd(); it++){
        if(it->IsString())
            l<<QString(it->AsStringUnchecked());
        else if(it->IsFloat())
            l<<(double)it->AsFloatUnchecked();
        else if(it->IsDouble())
            l<<it->AsDoubleUnchecked();
        else if(it->IsChar())
            l<<QChar(it->AsCharUnchecked());
        else if(it->IsBool())
            l<<it->AsBoolUnchecked();
        else if(it->IsInt32())
            l<<(int)it->AsInt32Unchecked();
        else if(it->IsInt64())
            l<<it->AsInt64Unchecked();
    }
	return l;
}

void sendMsg(IpEndpointName endpoint, QString oscAddr, QVariantList args)
{
	UdpTransmitSocket transmitSocket( endpoint );
	
    char buffer[OUTPUT_BUFFER_SIZE];
    osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
    
    p << osc::BeginBundleImmediate
	  << osc::BeginMessage( oscAddr.toStdString().data() );
   
	foreach (QVariant v, args) 
		sendToOscStream(p,v);
	
    p << osc::EndMessage
	  << osc::EndBundle;
    
	transmitSocket.Send( p.Data(), p.Size() );
}

bool sendMsg(QString ipAddress, int port, QString oscAddr, QVariantList args)
{
	IpEndpointName endpoint(ipAddress.toStdString().data(), port);
	if(!endpoint.address)
		return false;
	
	sendMsg(endpoint, oscAddr, args);
	return true;
}

void sendMsg(unsigned int ipAddress, int port, QString oscAddr, QVariantList args)
{
	sendMsg(IpEndpointName(ipAddress, port), oscAddr, args);
}

void sendMsg(int port, QString oscAddr, QVariantList args)
{
	sendMsg(IpEndpointName(IpEndpointName::ANY_ADDRESS, port), oscAddr, args);
}

bool sendToOscStream(osc::OutboundPacketStream &stream, const QVariant& v)
{
	switch(v.type()){
	case QVariant::Bool:
		stream<<v.toBool();
		break;
	case QVariant::Int:
		stream<<v.toInt();
		break;
	case QVariant::UInt:
		stream<<(int)v.toUInt();
		break;
	case QVariant::Double:
		stream<<v.toDouble();
		break;
	case QVariant::Char:
		stream<<v.toChar().toLatin1();
		break;
	case QVariant::String:
		stream<<v.toString().toStdString().data();
		break;
	case QVariant::StringList:
		foreach(QString s, v.toStringList())
			stream<<s.toStdString().data();
		break;
	default:
		return false;
	}

	return true;
}

} //namespace osc
