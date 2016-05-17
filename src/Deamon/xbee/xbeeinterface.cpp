#include "xbeeinterface.h"

#include <string.h>
#include <vector>
#include <QDebug>
#include <iostream>

#include "model/networkmodel.h"
#include "misc.h"

#ifdef POSIX
#include <sys/ioctl.h>
#include <fcntl.h>
#include <QDir>
#endif

XBeeInterface::XBeeInterface() : QThread()
{
    _state = DISCONNECTED;
}

/***********************************
 *        INTERFACE CONTROL        *
 **********************************/

std::vector<std::string> XBeeInterface::listPort() const
{
    std::vector<std::string> r;
#ifdef POSIX
    QStringList l = QDir("/dev","ttyUSB*",QDir::Name,QDir::System).entryList();

    foreach(QString s, l)
        r.push_back("/dev/"+s.toStdString());

#endif

    return r;
}

void XBeeInterface::scanNetwork()
{
    sendAT("ND", handleScanResponse);
    _scanNeeded = false;
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


/***********************************
 *        THREAD HANDLING          *
 **********************************/


bool XBeeInterface::tryToConnect()
{
    if(_forcePort)
        return tryToConnectOnPort(_port);

    std::vector<std::string> ports;
    ports = listPort();

    for(auto port=ports.begin(); port!=ports.end(); port++){
        if(*port != _port && tryToConnectOnPort(*port))
            return true;
    }
    if(_port=="")
        return false;
    return tryToConnectOnPort(_port);
}

bool XBeeInterface::tryToConnectOnPort(std::string port)
{
    xbee_serial_t serPort = { 9600, 0};
#ifdef POSIX
   strcpy(serPort.device,port.data());
#endif
   int status;
   if( (status = xbee_dev_init(&_xbee, &serPort, 0, 0)) ){
       if(status == -EIO)
           qWarning()<<"Wrong baudrate";
       return false;
   }

   if(xbee_cmd_init_device(&_xbee))
      return false;

   do {
       xbee_dev_tick( &_xbee);
       status = xbee_cmd_query_status( &_xbee);
   } while (status == -EBUSY);
    _port = port;
   return true;
}


bool XBeeInterface::initialize()
{
    int init = 0;

    _mac.clear();
    for(int i=0;i<8;i++)
        _mac.push_back(_xbee.wpan_dev.address.ieee.b[i]);

    while(init){
        xbee_dev_tick(&_xbee);
        msleep(200);
    }
    return true;
}

void XBeeInterface::standardRun()
{
    _frameStep = 0;
    qWarning()<<QString("Connected on port")+_xbee.serport.device;
    SNetworkModel::ptr()->setXbeeUsbPort(_xbee.serport.device);
    scanNetwork();
    while(_state == CONNECTED){
        if(!isStillConnected()){
            qWarning()<<"Disconnected";
            _state = DISCONNECTED;

        }

        if(!_frameStep){
            if(_scanNeeded)
                scanNetwork();
            else
                _scanNeeded = true;
        }


        xbee_dev_tick(&_xbee);
        if(_frameStep==5*FRAMERATE)
            _frameStep = 0;
        else
            _frameStep++;
        msleep(1000/FRAMERATE);
    }

    _frameStep = -1;
}

void XBeeInterface::forcePort(std::string port)
{
    if(port==""){
        _state = DISCONNECTED;
        _forcePort = false;
        return;
    }else
        _forcePort=true;
    if(_port==port || port == _xbee.serport.device)
        return;

    _port = port;
}


/***********************************
 *          CMD HANDLING           *
 **********************************/

bool XBeeInterface::sendRemoteAT(std::string cmd, const uint8_t dest[], std::function<int(std::vector<uint8_t>)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    const addr64* addr = new addr64({{dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6],dest[7]}});

    xbee_cmd_set_target(c, addr, 256*0xFF + 0xFE);

    return xbee_cmd_send(c)==0;
}


bool XBeeInterface::sendAT(std::string cmd, std::function<int(std::vector<uint8_t>)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    return xbee_cmd_send(c)==0;
}

int XBeeInterface::prepareXBeeATCmd(std::string cmd, std::function<int (std::vector<uint8_t>)> cb)
{
    xbee_cmd_callback_fn cmdCb =  [](const xbee_cmd_response_t *rep) -> int {
        if( (rep->flags&XBEE_CMD_RESP_MASK_STATUS) == XBEE_AT_RESP_ERROR){
            std::vector<uint8_t> addrToRemove;
            for(int i=0;i<8;i++)
                addrToRemove.push_back(rep->source->ieee.b[i]);
            SXBeeInterface::ptr()->removeRemote(addrToRemove);
            return XBEE_ATCMD_DONE;
        }

        std::function<int(std::vector<uint8_t>)> *cb = static_cast<std::function<int(std::vector<uint8_t>)>* >(rep->context);
        std::vector<uint8_t> d;
        for(int i=0; i<rep->value_length; i++)
            d.push_back(rep->value_bytes[i]);
        int r = (*cb)(d);
        if(r==XBEE_ATCMD_DONE)
            delete cb;
        return r;
    };
    return prepareXBeeATCmd(cmd, cmdCb, new std::function<int(std::vector<uint8_t>)>(cb));
}

int XBeeInterface::prepareXBeeATCmd(std::string cmd, xbee_cmd_callback_fn& cb, void* user_data)
{
    if(cmd.length() < 2)
        return -1;
    char at[3] = {cmd.at(0), cmd.at(1), '\0'};
    int c = xbee_cmd_create(&_xbee, at);

    xbee_cmd_set_callback(c, cb, user_data);

    std::vector<uint8_t> args = hexStrToInt(cmd.substr(2));
    xbee_cmd_set_param_bytes(c, args.data(), args.size());

    xbee_cmd_set_flags(c, XBEE_CMD_FLAG_REMOTE);

    return c;
}


bool XBeeInterface::isStillConnected()
{
    std::vector<std::string> ports = listPort();
    for(auto it=ports.begin(); it!=ports.end(); it++){
        if(*it == _port )
            return true;
    }
    return false;
}

/***********************************
 *          CMD HANDLING           *
 **********************************/

int XBeeInterface::handleScanResponse(std::vector<uint8_t> response)
{
    if(response.size()<10)
        return XBEE_ATCMD_DONE;
    std::vector<uint8_t> addr({response[2],response[3],response[4],response[5],response[6],response[7],response[8],response[9]});
    std::vector<XBeeRemote>& remotes= SXBeeInterface::ptr()->_remotes;

    for (size_t i = 0; i < remotes.size(); ++i)
        if(remotes[i].address()==addr)
            return XBEE_ATCMD_REUSE;

    SXBeeInterface::ptr()->addRemote(addr);

    return XBEE_ATCMD_REUSE;
}

bool XBeeInterface::addRemote(std::vector<uint8_t> addr)
{
    for (int i = 0; i < _remotes.size(); ++i) {
        if(_remotes.at(i).address()==addr)
            return false;
    }

    XBeeRemote r(addr, SXBeeInterface::ptr());
    _remotes.push_back(r);
    r.init();
    QMetaObject::invokeMethod(SNetworkModel::ptr()->remotes(), "addRemote", Q_ARG(QString, QString::fromStdString(intToHexStr(addr))) );
    qWarning()<<"remote added";
    return true;
}

bool XBeeInterface::removeRemote(std::vector<uint8_t> addr)
{
    for(auto it=_remotes.begin(); it!=_remotes.end(); it++){
        if(it->address()==addr){
            _remotes.erase(it);
            QMetaObject::invokeMethod(SNetworkModel::ptr()->remotes(), "removeRemote", Q_ARG(QString, QString::fromStdString(intToHexStr(addr))) );
            return true;
        }
    }
    return false;
}


const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
    XBEE_FRAME_HANDLE_LOCAL_AT,
    XBEE_FRAME_HANDLE_REMOTE_AT,
    XBEE_FRAME_TABLE_END
};
