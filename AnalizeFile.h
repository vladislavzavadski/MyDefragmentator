#include <Windows.h>
#include <iostream>
using namespace std;

typedef struct FILE_INFO{
	HANDLE hFile;
	RETRIEVAL_POINTERS_BUFFER buffer;
};

FILE_INFO* Get_RETRIEVAL_POINTERS_BUFFER_Of_File(WCHAR* pathToFile){
	FILE_INFO *temp;
	STARTING_VCN_INPUT_BUFFER SVIB;
	RETRIEVAL_POINTERS_BUFFER RPB;
	DWORD bytes;
	temp = (FILE_INFO*)malloc(sizeof(FILE_INFO));
	temp->hFile = CreateFile(pathToFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_READ_ATTRIBUTES, NULL);
	if (temp->hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}
	SVIB.StartingVcn.QuadPart = 0;
	if (!DeviceIoControl(temp->hFile, FSCTL_GET_RETRIEVAL_POINTERS, &SVIB, sizeof(SVIB), &RPB, sizeof(RPB), &bytes, NULL)){
		cout << "Error " << GetLastError()<<endl;
		system("pause");
		return NULL;
	}
	else{
		temp->buffer = RPB;
	}
	return temp;
}