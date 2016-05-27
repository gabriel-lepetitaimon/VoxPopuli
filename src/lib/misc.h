#ifndef MISC_SAFE_H
#define MISC_SAFE_H

#include <string>
#include <vector>


char intToHex(uint8_t value);
uint8_t hexToInt(char value);

std::string intToHexStr(std::vector<uint8_t> v);

std::vector<uint8_t> hexStrToInt(std::string str);

class HexData{
    std::vector<uint8_t> _data;
public:
    HexData(std::string data);
    HexData(std::vector<uint8_t> data);
    //static fromHexStr(std::string data);

    std::string toHexStr() const {return intToHexStr(_data);}
    std::string strData() const;
    const std::vector<uint8_t>& data() const {return _data;}
};


#endif // MISC_SAFE_H
