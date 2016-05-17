#include "xbeeremote.h"
#include "xbeeinterface.h"
#include "misc.h"

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

bool XBeeRemote::sendRX(std::string cmd) const
{
    return _interface->sendRemoteTX(cmd, _addr.data());
}

