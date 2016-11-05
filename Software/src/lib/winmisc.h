#ifndef WINMISC_H
#define WINMISC_H

#include <string>
#include <windows.h>

wchar_t* wcstring(std::string std, size_t convertedChars){
	wchar_t * wcstring = new wchar_t[std.length()+1];  
	mbstowcs_s(&convertedChars, wcstring, std.length()+1, std.data(), _TRUNCATE);  
	return wcstring;
}

wchar_t* wcstring(std::string std){
	size_t convertedChars=0;
	return wcstring(std,convertedChars);
}


#endif // WINMISC_H
