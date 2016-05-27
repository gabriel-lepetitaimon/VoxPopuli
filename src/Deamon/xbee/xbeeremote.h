#ifndef XBEEREMOTE_H
#define XBEEREMOTE_H

#include <functional>
#include <map>
#include <vector>
#include "xbee/atcmd.h"

class XBeeInterface;
class Remote;

enum XBEE_MSG_TYPE{
    LED_INTENSITY = 0x21,
    MUTE_MODE = 0x64,
    ACTIVE_MODE = 0x96,
    BUTTON_LEFT = 0xB0,
    BUTTON_UP = 0xB1,
    BUTTON_RIGHT = 0xB2,
    BUTTON_DOWN = 0xB3,
    BUTTON_ACTION = 0xB4,
    FRAME_END = 0xFF
};

class XBeeRemote
{

    std::vector<uint8_t> _addr;
    XBeeInterface* _interface;
    Remote* _remoteModel = 0;
    std::string rxBuffer;
    std::map<std::string, std::string> _safeSendMap;
public:
    XBeeRemote(std::vector<uint8_t> address, XBeeInterface* interface);

    void init();
    bool sendAT(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb= [](std::vector<uint8_t>){return true;}) const;
    bool sendTX(std::string cmd) const;
    bool sendTX(XBEE_MSG_TYPE cmd) const {return sendTX(std::string()+(char)cmd);}

    void receiveRX(std::string cmd);
    void handleMessage(std::string cmd);

    void sendMsg(XBEE_MSG_TYPE type, std::string data);
    void sendMsg(XBEE_MSG_TYPE type, std::vector<uint8_t> data = std::vector<uint8_t>());

    void safeSendMsg(std::string key, XBEE_MSG_TYPE type, std::string data);
    void safeSendMsg(std::string key, XBEE_MSG_TYPE type, std::vector<uint8_t> data = std::vector<uint8_t>());

    void checkStatus();
    bool tick();

    const std::vector<uint8_t>& address() const {return _addr;}
    Remote* remoteModel();

    void clearRemoteModel(){_remoteModel = 0;}

protected:
    std::string prepareMsg(XBEE_MSG_TYPE type, std::vector<uint8_t> data);
};

#endif // XBEEREMOTE_H

