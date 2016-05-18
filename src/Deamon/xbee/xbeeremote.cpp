#include "xbeeremote.h"
#include "xbeeinterface.h"
#include "misc.h"

#include <iostream>

XBeeRemote::XBeeRemote(std::vector<uint8_t> address, XBeeInterface *interface)
    :_addr(address), _interface(interface)
{

}

void XBeeRemote::init()
{
    std::string hexAddr = intToHexStr(_interface->macAddress());
    sendAT(std::string("DH"+hexAddr.substr(0,8)));
    sendAT(std::string("DL"+hexAddr.substr(8,8)));
}

bool XBeeRemote::sendAT(std::string cmd, std::function<int (std::vector<uint8_t>)> cb) const
{
    return _interface->sendRemoteAT(cmd, _addr.data(), cb);
}

bool XBeeRemote::sendTX(std::string cmd) const
{
    return _interface->sendRemoteTX(cmd, _addr.data());
}

void XBeeRemote::receiveRX(std::string cmd) const
{
    std::cout<<"Receive TX: "<<cmd<<std::endl;
}

void XBeeRemote::sendMsg(XBEE_MSG_TYPE type, std::string data)
{
    std::string cmd;
    cmd += type;
    cmd += data;
    cmd += FRAME_END;
    sendTX(cmd);
}

void XBeeRemote::sendMsg(XBEE_MSG_TYPE type, std::vector<uint8_t> data)
{
    std::string str = intToHexStr(data);
    return sendMsg(type, str);
}

