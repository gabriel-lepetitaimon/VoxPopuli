#ifndef MISC_SAFE_H
#define MISC_SAFE_H

#include <string>
#include <vector>


char intToHex(uint8_t value);
uint8_t hexToInt(char value);

std::string intToHexStr(std::vector<uint8_t> v);

std::vector<uint8_t> hexStrToInt(std::string str);


#endif // MISC_SAFE_H
