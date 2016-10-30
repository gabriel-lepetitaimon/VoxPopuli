#ifndef MISC_SAFE_H
#define MISC_SAFE_H

#include <string>
#include <vector>
#include <stdint.h>


char intToHex(uint8_t value);
uint8_t hexToInt(char value);

std::string intToHexStr(const std::vector<uint8_t> &v);
std::vector<uint8_t> hexStrToInt(const std::string& str);

class HexData{
public:
    HexData();
    HexData(const std::string &data);
    HexData(const std::vector<uint8_t> &data);
    HexData(const char* data, const unsigned int size=1);
    HexData(const uint8_t data);
    HexData(const HexData& data);
    ~HexData();

    static HexData fromHexStr(std::string data);

    std::string toHexStr() const;
    std::string toIntStr(char sep = ' ') const;
    std::string toCharStr() const;
    std::string strData() const;
    unsigned int toInt() const;
    bool toInt(unsigned int& r) const;
    const std::vector<uint8_t> &data() const;

    uint8_t byteAt(int i) const {return (*_data)[i];}
    uint8_t& operator[](size_t i) {return(*_data)[i];}
    const uint8_t& operator[](size_t i) const {return (*_data)[i];}
    size_t size() const {return _data->size();}

    void prepend(const std::vector<uint8_t>& data);
    void prepend(uint8_t data);
    void prepend(const std::string& data){ prepend(hexStrToInt(data));}

    void append(const std::vector<uint8_t>& data);
    void append(uint8_t data);
    void append(const std::string& data){ append(hexStrToInt(data));}

    HexData operator+=(const HexData& data);
    HexData operator+=(const std::vector<uint8_t>& data);
    HexData operator+=(const std::string& data);
    HexData operator+=(uint8_t data);


    HexData operator+(const HexData& data) const;
    HexData operator+(uint8_t data) const;

protected:
    HexData(std::vector<uint8_t>* emplace);
    std::vector<uint8_t>* _data;

};


#endif // MISC_SAFE_H
