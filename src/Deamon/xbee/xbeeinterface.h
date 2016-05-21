#ifndef XBEEINTERFACE_H
#define XBEEINTERFACE_H

#include <QThread>
#include <functional>
#include <string>
#include <vector>
#include "xbee/platform.h"
#include "xbee/byteorder.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include "wpan/types.h"


#include "singleton.h"

#include "xbeeremote.h"

#define FRAMERATE 200

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

     static int xbeeTX( xbee_dev_t *xbee, const void FAR *raw, uint16_t length, void FAR *context);
     void forcePort(std::string port);

     bool sendRemoteAT( std::string cmd, const uint8_t dest[9], std::function<bool(std::vector<uint8_t>)> cb = [](std::vector<uint8_t>){return true;});
     bool sendAT(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb= [](std::vector<uint8_t>){return true;});

     bool sendRemoteTX(std::string cmd, const uint8_t dest[9]);

     std::vector<std::string> listPort() const;
     void scanNetwork();

     std::vector<XBeeRemote>& remotes() {return _remotes;}
     XBeeRemote *remote(const uint8_t dest[9]);
     const std::vector<uint8_t>& macAddress() const {return _mac;}

protected:
    void run();

    bool tryToConnect();
    bool initialize();
    void standardRun();



private:
    xbee_dev_t _xbee;
    int _scanCmd=-1;
    XBeeState _state;
    std::vector<XBeeRemote> _remotes;
    std::vector<uint8_t> _mac;


    int prepareXBeeATCmd(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb);
    int prepareXBeeATCmd(std::string cmd, xbee_cmd_callback_fn& cb, void* user_data);
    bool tryToConnectOnPort(std::string port);
    bool isStillConnected();

    bool addRemote(std::vector<uint8_t> addr);
    bool removeRemote(std::vector<uint8_t> addr);
    static bool handleScanResponse(std::vector<uint8_t> response);

    int16_t generateFrameID(int recursion=1) const;

    std::string _port;
    bool _forcePort=false;
    int _frameStep=-1;
    bool _scanNeeded=true;    

};
#endif // XBEEINTERFACE_H
