#include "xbeeremote.h"
#include "xbeeinterface.h"
#include "model/networkmodel.h"

#include <iostream>

XBeeRemote::XBeeRemote(std::vector<uint8_t> address, XBeeInterface *interface)
    :_addr(address), _interface(interface)
{

}

void XBeeRemote::init()
{
    std::string hex = HexData(_interface->macAddress()).toHexStr();
    sendAT(std::string("DH"+hex.substr(0,8)));
    sendAT(std::string("DL"+hex.substr(8,8)));
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
        remoteModel()->setButtonState(BUTTON_LEFT, cmd[1]);
        break;
    case BUTTON_RIGHT:
        remoteModel()->setButtonState(BUTTON_RIGHT, cmd[1]);
        break;
    case BUTTON_UP:
        remoteModel()->setButtonState(BUTTON_UP, cmd[1]);
        break;
    case BUTTON_DOWN:
        remoteModel()->setButtonState(BUTTON_DOWN, cmd[1]);
        break;
    case BUTTON_ACTION:
            remoteModel()->setButtonState(BUTTON_ACTION, cmd[1]);
            break;
    default:
        return;
    }
}

void XBeeRemote::sendMsg(XBEE_MSG_TYPE type, const HexData &data)
{
    sendTX(prepareMsg(type, data));
}

void XBeeRemote::safeSendMsg(std::string key, XBEE_MSG_TYPE type, const HexData &data)
{
    _safeSendMap[key] = prepareMsg(type, data);
}

std::string XBeeRemote::prepareMsg(XBEE_MSG_TYPE type, const HexData& data)
{
    std::string cmd;
    cmd+=type;
    cmd+=data.strData();
    cmd += FRAME_END;
    return cmd;
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

bool XBeeRemote::tick()
{
    if(_safeSendMap.empty())
        return false;
    auto it = _safeSendMap.begin();
    sendTX(it->second);
    _safeSendMap.erase(it);
    return true;
}

Remote *XBeeRemote::remoteModel()
{
    if(_remoteModel)
        return _remoteModel;
    _remoteModel = SNetworkModel::ptr()->remotes()->byAddr(QString::fromStdString(intToHexStr(_addr)));
    return _remoteModel;
}
