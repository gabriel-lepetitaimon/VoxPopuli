#include "xbeeremote.h"
#include "xbeeinterface.h"
#include "model/networkmodel.h"
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

bool XBeeRemote::sendAT(std::string cmd, std::function<bool (std::vector<uint8_t>)> cb) const
{
    return _interface->sendRemoteAT(cmd, _addr.data(), cb);
}

bool XBeeRemote::sendTX(std::string cmd) const
{
    return _interface->sendRemoteTX(cmd, _addr.data());
}

void XBeeRemote::receiveRX(std::string cmd)
{
    cmd = rxBuffer + cmd;
    std::string buf;
    std::vector<std::string> msgs;
    for (size_t i = 0; i < cmd.size(); ++i) {

        if((uint8_t)cmd[i] == FRAME_END){
            if(buf=="")
                continue;
            msgs.push_back(buf);
            buf = "";
        }else
            buf+=cmd[i];
    }
    rxBuffer = buf;
    for(size_t i=0; i<msgs.size(); i++)
        handleMessage(msgs[i]);
}

void XBeeRemote::handleMessage(std::string cmd)
{
    switch((uint8_t)cmd[0]){
    case BUTTON_LEFT:
        remoteModel()->setButtonState(Remote::LEFT, cmd[1]);
        break;
    case BUTTON_RIGHT:
        remoteModel()->setButtonState(Remote::RIGHT, cmd[1]);
        break;
    case BUTTON_UP:
        remoteModel()->setButtonState(Remote::UP, cmd[1]);
        break;
    case BUTTON_DOWN:
        remoteModel()->setButtonState(Remote::DOWN, cmd[1]);
        break;
    case BUTTON_ACTION:
            remoteModel()->setButtonState(Remote::ACTION, cmd[1]);
            break;
    default:
        return;
    }
}

void XBeeRemote::sendMsg(XBEE_MSG_TYPE type, std::string data)
{
    std::vector<uint8_t> hex = hexStrToInt(data);
    sendMsg(type, hex);
}

void XBeeRemote::sendMsg(XBEE_MSG_TYPE type, std::vector<uint8_t> data)
{
    std::string cmd;
    cmd+=type;
    for(size_t i=0; i<data.size(); i++)
        cmd+=data[i];
    cmd += FRAME_END;
    sendTX(cmd);
}

void XBeeRemote::checkStatus()
{
//    sendAT("DB",
//           [this](std::vector<uint8_t> d){
//                if(!d.empty())
//                    remoteModel()->setSignalStrength(-d.at(0));
//                return true;
//            }
//    );
}

Remote *XBeeRemote::remoteModel()
{
    if(_remoteModel)
        return _remoteModel;
    _remoteModel = SNetworkModel::ptr()->remotes()->byAddr(QString::fromStdString(intToHexStr(_addr)));
    return _remoteModel;
}
