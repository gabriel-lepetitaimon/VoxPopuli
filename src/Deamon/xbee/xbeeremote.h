#ifndef XBEEREMOTE_H
#define XBEEREMOTE_H

#include <functional>
#include <vector>
#include "xbee/atcmd.h"

class XBeeInterface;
class Remote;

enum XBEE_MSG_TYPE{
    LED_ON = 20,
    LED_OFF = 10,
    FRAME_END = 255
};

class XBeeRemote
{

    std::vector<uint8_t> _addr;
    XBeeInterface* _interface;
    Remote* _remoteModel = 0;
public:
    XBeeRemote(std::vector<uint8_t> address, XBeeInterface* interface);

    void init();
    bool sendAT(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb= [](std::vector<uint8_t>){return true;}) const;
    bool sendTX(std::string cmd) const;

    void receiveRX(std::string cmd) const;

    void sendMsg(XBEE_MSG_TYPE type, std::string data="");
    void sendMsg(XBEE_MSG_TYPE type, std::vector<uint8_t> data);

    void checkStatus();

    const std::vector<uint8_t>& address() const {return _addr;}
    Remote* remoteModel();

    void clearRemoteModel(){_remoteModel = 0;}
};

#endif // XBEEREMOTE_H
