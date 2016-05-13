#include "xbeeinterface.h"

#include <string.h>
#include <QDebug>

#include "model/networkmodel.h"

#ifdef POSIX
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

XBeeInterface::XBeeInterface() : QThread()
{
    _state = DISCONNECTED;
}

void XBeeInterface::run()
{
    while(1){
        switch(_state){
        case XBeeInterface::DISCONNECTED:
            if(tryToConnect()){
                _state = INITIALIZING;
                continue;
            }
            break;
        case XBeeInterface::INITIALIZING:
            if(initialize()){
                _state = CONNECTED;
                continue;
            }else{
                _state = DISCONNECTED;
                continue;
            }
            break;
        case XBeeInterface::CONNECTED:
            standardRun();
            _state = DISCONNECTED;
            continue;
        }
        msleep(1000);
    }
}

bool XBeeInterface::tryToConnect()
{
    int i=0;
    while(1){
#ifdef POSIX
        if(i>30)
            return false;
        const char* path;
        if(_forcePort!=""){
            if(i>0)
                return false;
            path = _forcePort.data();
        }else
            path = (std::string("/dev/ttyUSB")+std::to_string(i)).data();
        xbee_serial_t serPort = { 9600, 0};
        strcpy(serPort.device,path);
        i++;
#endif

        int status;
        if( (status = xbee_dev_init(&_xbee, &serPort, 0, 0)) ){
            if(status == -EIO)
                qWarning()<<"Wrong baudrate";
            continue;
        }
        break;
    }
    return !xbee_cmd_init_device(&_xbee);
}

bool XBeeInterface::initialize()
{
    int status;
    do {
        xbee_dev_tick( &_xbee);
        status = xbee_cmd_query_status( &_xbee);
    } while (status == -EBUSY);

    return true;
}

void XBeeInterface::standardRun()
{
    qWarning()<<QString("Connected on port")+_xbee.serport.device;
    SNetworkModel::ptr()->setXbeeUsbPort(_xbee.serport.device);
    sendAT("ND");
    while(1){
        if(_state!=CONNECTED)
            return;
        xbee_cmd_tick();
        msleep(5);
    }
}

void XBeeInterface::forcePort(std::string port)
{
    if(_forcePort==port || port == _xbee.serport.device)
        return;

    if(port!="")
        _state = DISCONNECTED;
    _forcePort = port;
}


/***********************************
 *          CMD HANDLING           *
 **********************************/

bool XBeeInterface::sendRemoteAT( std::string cmd, char dest[9], std::function<int(std::string)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    const addr64* addr = new addr64({{dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6],dest[7]}});

    xbee_cmd_set_target(c, addr, 0);

    return xbee_cmd_send(c)==0;

}

bool XBeeInterface::sendAT(std::string cmd, std::function<int(std::string)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    return xbee_cmd_send(c)==0;
}

int XBeeInterface::prepareXBeeATCmd(std::string cmd, std::function<int (std::string)> cb)
{
    if(cmd.length() < 2)
        return -1;
    char at[3] = {cmd.at(0), cmd.at(1), '\0'};
    int c = xbee_cmd_create(&_xbee, at);

    xbee_cmd_callback_fn cmdCb =  [](const xbee_cmd_response_t *rep) -> int {
        std::function<int(std::string)> *cb = static_cast<std::function<int(std::string)>* >(rep->context);
        char d[rep->value_length];
        for(int i=0; i<rep->value_length; i++)
            d[i] = (char)rep->value_bytes[i];
        std::string str(d);
        int r = (*cb)(str);
        delete cb;
        return r;
    };
    xbee_cmd_set_callback(c, cmdCb, new std::function<int(std::string)>(cb));


    xbee_cmd_set_param_bytes(c, cmd.substr(2).data(), cmd.size()-2);

    return c;
}

int XBeeInterface::xbeeATResponse(xbee_dev_t *xbee, const void *raw, uint16_t length, void *context)
{

    return 0;
}

const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
    XBEE_FRAME_HANDLE_LOCAL_AT,
    { XBEE_FRAME_LOCAL_AT_RESPONSE, 0, XBeeInterface::xbeeATResponse, NULL },
    XBEE_FRAME_TABLE_END
};
