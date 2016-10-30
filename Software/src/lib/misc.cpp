#include "misc.h"

using namespace std;

char intToHex(uint8_t value){
    switch(value){
    case 0:
        return '0';
    case 1:
        return '1';
    case 2:
        return '2';
    case 3:
        return '3';
    case 4:
        return '4';
    case 5:
        return '5';
    case 6:
        return '6';
    case 7:
        return '7';
    case 8:
        return '8';
    case 9:
        return '9';
    case 10:
        return 'A';
    case 11:
        return 'B';
    case 12:
        return 'C';
    case 13:
        return 'D';
    case 14:
        return 'E';
    default:
        return 'F';
    }
    return 'F';
}

uint8_t hexToInt(char value){
    switch(value){
    case '1':
        return 1;
    case '2':
        return 2;
    case '3':
        return 3;
    case '4':
        return 4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'A':
        return 10;
    case 'B':
        return 11;
    case 'C':
        return 12;
    case 'D':
        return 13;
    case 'E':
        return 14;
    case 'F':
        return 15;
    default:
        return 0;
    }
    return 0;
}

string intToHexStr(const vector<uint8_t>& v){
    string str="";
    for (size_t i = 0; i < v.size(); ++i) {
        uint8_t d1 = v[i]/16;
        uint8_t d2 = v[i]%16;
        str+= intToHex(d1);
        str+= intToHex(d2);
    }

    return str;
}

vector<uint8_t> hexStrToInt(const string& str){
    vector<uint8_t> r;
    for(size_t i=0; i<str.length()/2; i++){
        uint8_t d = 0;
        d += hexToInt(str.at(2*i))*16;
        d += hexToInt(str.at(2*i+1));
        r.push_back(d);
    }

    return r;
}

/********************************************
 *                 Hex Data                 *
 *******************************************/


HexData::HexData()
{
    _data = new std::vector<uint8_t>();
}

HexData::HexData(const string &data)
{
    _data = new std::vector<uint8_t>(2*data.size());
    for(size_t i=0; i<data.size(); i++)
        (*_data)[i] = (uint8_t)data.at(i);
}

HexData::HexData(const std::vector<uint8_t> &data)
{
    _data = new std::vector<uint8_t>(data);
}

HexData::HexData(const char* data, const unsigned int size){
    _data = new std::vector<uint8_t>(size);
    for(unsigned int i=0; i<size; i++)
        (*_data)[i] = (uint8_t)data[i];
}

HexData::HexData(const uint8_t data)
{
    _data = new std::vector<uint8_t>(1, data);
}

HexData::HexData(const HexData &data)
{
    _data = new std::vector<uint8_t>(*(data._data));
}

HexData::HexData(std::vector<uint8_t> *emplace)
    :_data(emplace)
{
}

HexData::~HexData()
{
    delete _data;
}

HexData HexData::fromHexStr(string data)
{
    return HexData(hexStrToInt(data));
}

string HexData::toHexStr() const
{
    return intToHexStr(*_data);
}

string HexData::toIntStr(char sep) const
{
    string r;
    for(size_t i=0; i<_data->size(); i++){
        if(i) r += sep;
        r += std::to_string((*_data)[i]);
    }
    return r;
}


string HexData::strData() const
{
    string r;
    for(size_t i=0; i<_data->size(); i++)
        r.push_back((*_data)[i]);
    return r;
}

unsigned int HexData::toInt() const
{
    unsigned int r = 0;
    toInt(r);
    return r;
}

bool HexData::toInt(unsigned int &r) const
{
    if(size()>32)
        return false;
    r = 0;
    for(size_t i = _data->size()-1; i+1>0; i--){
        r *= 256;
        r += _data->at(i);
    }
    return true;
}

const std::vector<uint8_t>& HexData::data() const
{
    return *_data;
}

void HexData::prepend(const std::vector<uint8_t> &data)
{
    std::vector<uint8_t>* r = new std::vector<uint8_t>(data.size()+_data->size());
    for (size_t i = 0; i < data.size(); ++i)
        (*r)[i] = data[i];
    for (size_t i = 0; i < _data->size(); ++i)
        (*r)[i+data.size()] = data[i];

    delete _data;
    _data = r;
}

void HexData::prepend(uint8_t data)
{
    _data->insert(_data->begin(), data);
}

void HexData::append(const std::vector<uint8_t> &data)
{
    _data->reserve(_data->size()+data.size());
    for (size_t i = 0; i < data.size(); ++i)
        _data->insert(_data->end(), data[i]);
}

void HexData::append(uint8_t data)
{
    _data->insert(_data->end(), data);
}

HexData HexData::operator+=(const HexData &data)
{
    append(*(data._data));
    return *this;
}

HexData HexData::operator+=(const std::vector<uint8_t> &data)
{
    append(data);
    return *this;
}

HexData HexData::operator+=(const string &data)
{
    append(data);
    return *this;
}

HexData HexData::operator+=(uint8_t data)
{
    append(data);
    return *this;
}

HexData HexData::operator+(const HexData &data) const
{
    std::vector<uint8_t>* r = new std::vector<uint8_t>(data.size()+_data->size());
    for (size_t i = 0; i < _data->size(); ++i)
        (*r)[i+_data->size()] = (*_data)[i];
    for (size_t i = 0; i < data.size(); ++i)
        (*r)[i] = data[i];

    return HexData(r);
}

HexData HexData::operator+(uint8_t data) const
{
    std::vector<uint8_t>* r = new std::vector<uint8_t>(_data->size()+1);
    for (size_t i = 0; i < _data->size(); ++i)
        (*r)[i+_data->size()] = (*_data)[i];
    (*r)[_data->size()] = data;
    return HexData(r);
}




