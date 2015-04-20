#include <iostream>
#include <windows.h>

class Disk{
public:
	Disk();
	HANDLE hDisk;
	WCHAR *individualName;
	WCHAR *Letter;
};