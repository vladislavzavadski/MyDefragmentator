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
	temp->hFile = CreateFile(pathToFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
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

LARGE_INTEGER GetStartLcn(FILE_INFO file, PVOLUME_BITMAP_BUFFER vbb){
	LARGE_INTEGER StartLcn;
	RETRIEVAL_POINTERS_BUFFER buf =  file.buffer;
	unsigned long long int count = 0;
	bool chainIsSearched = false;
	StartLcn.QuadPart = 0;
	unsigned long long int clustersLength = buf.Extents[0].NextVcn.QuadPart - buf.StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < vbb->BitmapSize.QuadPart; i++){
		if (vbb->Buffer[i] == 0 && chainIsSearched == true){
			count++;
		}
		if (vbb->Buffer[i] == 0 && chainIsSearched == false){
			count++;
			StartLcn.QuadPart = i;
			chainIsSearched == true;
		}
		if (vbb->Buffer[i] != 0){
			chainIsSearched = false;
			count = 0;
		}

		if (count == clustersLength){
			break;
		}

	}
	return StartLcn;
}

bool DefragmentateFile(FILE_INFO file, PVOLUME_BITMAP_BUFFER vbb, Disk drive){
	MOVE_FILE_DATA movFileStruct;
	DWORD pBytes;
	movFileStruct.FileHandle = file.hFile;
	movFileStruct.StartingVcn.QuadPart = 0;
	movFileStruct.StartingLcn = GetStartLcn(file, vbb);
	movFileStruct.ClusterCount = file.buffer.Extents[0].NextVcn.QuadPart - file.buffer.StartingVcn.QuadPart;
	wcout << drive.Letter;
	bool ret = DeviceIoControl(drive.hDisk, FSCTL_MOVE_FILE, &movFileStruct, sizeof(MOVE_FILE_DATA), NULL, 0, &pBytes, NULL);
	if (!ret && GetLastError() == ERROR_ACCESS_DENIED){
		cout << "nu emae"<<GetLastError();
	}
	else{
		cout << "yes";
	}
	return true;
}