#define XBEE_CMD_REQUEST_TABLESIZE	3
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

XBeeRemote *XBeeInterface::remote(const uint8_t dest[])
{
    for(auto it = _remotes.begin(); it!=_remotes.end(); it++){
        for(int i=0; i<8;i++){
            if(it->address().at(i) != dest[i])
                break;
            if(i==7)
                return &(*it);
        }
    }
    return 0;
}


/***********************************
 *        THREAD HANDLING          *
 **********************************/

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
            if(!_forcePort && SNetworkModel::ptr())
                SNetworkModel::ptr()->set("USBPort", "");
            continue;
        }
        msleep(1000);
    }
}

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
            else{
                //_scanNeeded = true;
                for (size_t i = 0; i < _remotes.size(); ++i) {
                    _remotes.at(i).checkStatus();
                }
            }
        }


        xbee_dev_tick(&_xbee);

        if(_fastCycle){
            _fastCycle = false;
           // continue;
        }

        for(size_t i=0; i<_remotes.size(); i++){
            if(_remotes[i].tick())
                break;
        }


        if(_frameStep==5*FRAMERATE)
            _frameStep = 0;
        else
            _frameStep++;
        msleep(2500/FRAMERATE);

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
 *        AT CMD HANDLING          *
 **********************************/

//----------  SEND   ---------------

bool XBeeInterface::sendRemoteAT(std::string cmd, const uint8_t dest[], std::function<bool(std::vector<uint8_t>)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    const addr64* addr = new addr64({{dest[0], dest[1], dest[2], dest[3], dest[4], dest[5], dest[6],dest[7]}});

    xbee_cmd_set_target(c, addr, 256*0xFF + 0xFE);

    return xbee_cmd_send(c)==0;
}


bool XBeeInterface::sendAT(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb)
{
    int c = prepareXBeeATCmd(cmd, cb);
    if(c==-1)
        return false;

    return xbee_cmd_send(c)==0;
}

void XBeeInterface::scanNetwork()
{
    if(_scanCmd == -1)
        _scanCmd = prepareXBeeATCmd("ND", handleScanResponse);

    xbee_cmd_send( (int16_t)_scanCmd);
    _scanNeeded = false;
}

int XBeeInterface::prepareXBeeATCmd(std::string cmd, std::function<bool(std::vector<uint8_t>)> cb)
{
    xbee_cmd_callback_fn cmdCb =  [](const xbee_cmd_response_t *rep) -> int {
        if( (rep->flags&XBEE_CMD_RESP_MASK_STATUS) == XBEE_AT_RESP_ERROR || (rep->flags&XBEE_CMD_RESP_MASK_STATUS) == XBEE_AT_RESP_TX_FAIL){
            std::vector<uint8_t> addrToRemove;
            for(int i=0;i<8;i++)
                addrToRemove.push_back(rep->source->ieee.b[i]);
            SXBeeInterface::ptr()->removeRemote(addrToRemove);
            return XBEE_ATCMD_DONE;
        }

        std::function<bool(std::vector<uint8_t>)> *cb = static_cast<std::function<bool(std::vector<uint8_t>)>* >(rep->context);
        std::vector<uint8_t> d;
        for(int i=0; i<rep->value_length; i++)
            d.push_back(rep->value_bytes[i]);
        bool r = (*cb)(d);
        if(r)
            delete cb;
        return r?XBEE_ATCMD_DONE:XBEE_ATCMD_REUSE;
    };
    return prepareXBeeATCmd(cmd, cmdCb, new std::function<bool(std::vector<uint8_t>)>(cb));
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
    _fastCycle = true;
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

//----------  RESPONSE  ------------

bool XBeeInterface::handleScanResponse(std::vector<uint8_t> response)
{
    if(response.size()<10)
        return false;
    std::vector<uint8_t> addr({response[2],response[3],response[4],response[5],response[6],response[7],response[8],response[9]});
    std::vector<XBeeRemote>& remotes= SXBeeInterface::ptr()->_remotes;

    if(addr == SXBeeInterface::ptr()->_mac)
        return false;

    for (size_t i = 0; i < remotes.size(); ++i)
        if(remotes[i].address()==addr)
            return false;

    SXBeeInterface::ptr()->addRemote(addr);

    return false;
}

bool XBeeInterface::addRemote(std::vector<uint8_t> addr)
{
    for (size_t i = 0; i < _remotes.size(); ++i) {
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
            qWarning()<<"remote removed";
            return true;
        }
    }
    return false;
}


/***********************************
 *           TX HANDLING           *
 **********************************/

//----------  SEND   ---------------

bool XBeeInterface::sendRemoteTX(std::string cmd, const uint8_t dest[])
{
    std::vector<uint8_t> header;
    int16_t c;

    header.push_back(XBEE_FRAME_TRANSMIT);          //frame type
    if( (c=generateFrameID()) !=-1) header.push_back(c);    //frame ID
    else return false;

    for(int i=0;i<8;i++) header.push_back(dest[i]); //address
    header.push_back(0xFF);         //16bit address
    header.push_back(0xFE);

    header.push_back(0x00);         //Broadcast radius
    header.push_back(0x00);         //Options

    _fastCycle = true;

    return !xbee_frame_write(&_xbee, header.data(), header.size(), cmd.data(), cmd.length(), XBEE_DEV_FLAG_NONE);
}

int16_t XBeeInterface::generateFrameID(int recursion) const
{
    xbee_cmd_request_t FAR *request;
    int_fast8_t index;
    request = xbee_cmd_request_table;
    for (index = 0; index < XBEE_CMD_REQUEST_TABLESIZE; ++request, ++index){
        if (request->device == NULL)
            break;
    }

    if(index == XBEE_CMD_REQUEST_TABLESIZE){
        if(!recursion)
            return -1;

        xbee_cmd_tick();
        return generateFrameID(recursion-1);
    }




    return (index << 8) | request->sequence;
}

//----------  RESPONSE  ------------

int XBeeInterface::xbeeTX(xbee_dev_t *, const void *raw, uint16_t length, void *)
{
    if(length<13)
        return XBEE_ATCMD_REUSE;

    const uint8_t *frame = static_cast<const uint8_t*>(raw);
    uint8_t addr[8] = {frame[1],frame[2],frame[3],frame[4],frame[5],frame[6],frame[7],frame[8]};
    XBeeRemote* remote = SXBeeInterface::ptr()->remote(addr);

    if(!remote)
        return XBEE_ATCMD_REUSE;

    std::string data="";
    for(int i=12;i<length;i++)
        data += frame[i];

     remote->receiveRX(data);

    return XBEE_ATCMD_REUSE;
}


/***********************************
 *          XBEE LIB PARAM         *
 **********************************/

const xbee_dispatch_table_entry_t xbee_frame_handlers[] =
{
    XBEE_FRAME_HANDLE_LOCAL_AT,
    XBEE_FRAME_HANDLE_REMOTE_AT,
    {XBEE_FRAME_RECEIVE, 0, XBeeInterface::xbeeTX, NULL},
    XBEE_FRAME_TABLE_END
};
