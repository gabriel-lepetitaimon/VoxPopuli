#ifndef SERIAL_CAST_H
#define SERIAL_CAST_H

#include <windows.h>

wchar_t* wcstring(char* std){

    size_t newsize = sizeof std + 1;  

    wchar_t * wcstring[newsize];  
    
    // Convert char* string to a wchar_t* string.  
    size_t convertedChars = 0;  
    mbstowcs_s(&convertedChars, wcstring, newsize, std, _TRUNCATE);  
    return wcstring;
}

#endif // SERIAL_CAST_H
