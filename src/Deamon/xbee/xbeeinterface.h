#ifndef XBEEINTERFACE_H
#define XBEEINTERFACE_H

#include <QThread>
#include <functional>

#include "xbee/platform.h"
#include "xbee/byteorder.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include "wpan/types.h"
#include "singleton.h"

class XBeeInterface;
typedef singleton<XBeeInterface> SXBeeInterface;

class XBeeInterface : public QThread
{
    Q_OBJECT
public:
    XBeeInterface();

    enum XBeeState{
        DISCONNECTED,
        INITIALIZING,
        CONNECTED
    };

     static int xbeeATResponse( xbee_dev_t *xbee, const void FAR *raw, uint16_t length, void FAR *context);
     void forcePort(std::string port);

protected:
    void run();

    bool tryToConnect();
    bool initialize();
    void standardRun();

    bool sendRemoteAT( std::string cmd, char dest[9], std::function<int(std::string)> cb = [](std::string){return XBEE_ATCMD_DONE;});
    bool sendAT(std::string cmd, std::function<int(std::string)> cb= [](std::string){return XBEE_ATCMD_DONE;});



private:
    xbee_dev_t _xbee;
    XBeeState _state;

    int prepareXBeeATCmd(std::string cmd, std::function<int(std::string)> cb);

    std::string _forcePort;
};

#endif // XBEEINTERFACE_H
