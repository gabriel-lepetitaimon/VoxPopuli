#ifndef XBEEREMOTE_H
#define XBEEREMOTE_H

#include <functional>
#include <vector>
#include "xbee/atcmd.h"

class XBeeInterface;

class XBeeRemote
{

    std::vector<uint8_t> _addr;
    XBeeInterface* _interface;
public:
    XBeeRemote(std::vector<uint8_t> address, XBeeInterface* interface);

    void init();
    bool sendAT(std::string cmd, std::function<int(std::vector<uint8_t>)> cb= [](std::vector<uint8_t>){return XBEE_ATCMD_DONE;}) const;
    bool sendRX(std::string cmd) const;

    const std::vector<uint8_t>& address() const {return _addr;}
};

#endif // XBEEREMOTE_H
