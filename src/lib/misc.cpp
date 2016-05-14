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

string intToHexStr(vector<uint8_t> v){
    string str="";
    for (size_t i = 0; i < v.size(); ++i) {
        uint8_t d1 = v[i]/16;
        uint8_t d2 = v[i]%16;
        str+= intToHex(d1);
        str+= intToHex(d2);
    }

    return str;
}

vector<uint8_t> hexStrToInt(string str){
    if(str.length()%2)
        str+='0';
    vector<uint8_t> r;
    for(size_t i=0; i<str.length()/2; i++){
        uint8_t d = 0;
        d += hexToInt(str.at(2*i))*16;
        d += hexToInt(str.at(2*i+1));
        r.push_back(d);
    }

    return r;
}
