#include <Windows.h>
#include <iostream>
using namespace std;

typedef struct FILE_INFO{
	HANDLE hFile;
	PRETRIEVAL_POINTERS_BUFFER buffer;
};

FILE_INFO* Get_RETRIEVAL_POINTERS_BUFFER_Of_File(WCHAR* pathToFile){
	FILE_INFO *temp;
	STARTING_VCN_INPUT_BUFFER SVIB;
	RETRIEVAL_POINTERS_BUFFER rpb;
	PRETRIEVAL_POINTERS_BUFFER RPB = &rpb;
	_int64 pOutSize = 0;
	DWORD bytes;
	temp = (FILE_INFO*)malloc(sizeof(FILE_INFO));
	temp->hFile = CreateFile(pathToFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (temp->hFile == INVALID_HANDLE_VALUE){
		return NULL;
	}
	SVIB.StartingVcn.QuadPart = 0;
	bool ret = DeviceIoControl(temp->hFile, FSCTL_GET_RETRIEVAL_POINTERS, &SVIB, sizeof(SVIB), RPB, sizeof(RETRIEVAL_POINTERS_BUFFER), &bytes, NULL);
	while (!ret&&GetLastError() == ERROR_MORE_DATA){
		pOutSize = pOutSize + (RPB->ExtentCount - RPB->StartingVcn.QuadPart) + sizeof(RETRIEVAL_POINTERS_BUFFER);
		RPB = (PRETRIEVAL_POINTERS_BUFFER)malloc(pOutSize);
		RPB->StartingVcn.QuadPart = 0;
		ret = DeviceIoControl(temp->hFile, FSCTL_GET_RETRIEVAL_POINTERS, &SVIB, sizeof(SVIB), RPB, pOutSize, &bytes, NULL);
		if (!ret&&GetLastError()!=ERROR_MORE_DATA){
			cout << "Error " << GetLastError() << endl;
			system("pause");
			return NULL;
		}


	}

		temp->buffer = RPB;

	return temp;
}


LARGE_INTEGER GetStartLcn(unsigned long long int clustersLength, PVOLUME_BITMAP_BUFFER vbb){
	LARGE_INTEGER StartLcn, EndLcn;
	unsigned long long int count = 0;
	bool chainIsSearched = false;
	StartLcn.QuadPart = 0;
	EndLcn.QuadPart = 0;
	for (unsigned long long int i = 0; i < vbb->BitmapSize.QuadPart; i++){
		if (vbb->Buffer[i] == 0 && chainIsSearched == true){
			count++;
		}
		if (vbb->Buffer[i] == 0 && chainIsSearched == false){
			count++;
			StartLcn.QuadPart = i;
			chainIsSearched = true;
		}
		if (vbb->Buffer[i] != 0){
			chainIsSearched = false;
			count = 0;
		}

		if (count == clustersLength){
			EndLcn.QuadPart = i;
			break;
		}

	}
	StartLcn.QuadPart *= 8;
	return StartLcn;
}

unsigned long long int GetFullClustersCount(FILE_INFO file){
	unsigned long long int fullClustersCount = file.buffer->Extents[0].NextVcn.QuadPart-file.buffer->StartingVcn.QuadPart;
	for (unsigned long long int i = 1; i < file.buffer->ExtentCount; i++){
		fullClustersCount += file.buffer->Extents[i].NextVcn.QuadPart - file.buffer->Extents[i - 1].NextVcn.QuadPart;
	}
	return fullClustersCount;
}

bool DefragmentateFile(FILE_INFO file, PVOLUME_BITMAP_BUFFER vbb, Disk drive){
	MOVE_FILE_DATA movFileStruct;
	unsigned long long int prevVcn;
	unsigned long long int fullClustersCount;
	LARGE_INTEGER lcn;
	DWORD pBytes = 0;
	if (file.buffer->ExtentCount == 1){
		return true;
	}
	movFileStruct.FileHandle = file.hFile;
	movFileStruct.StartingVcn = file.buffer->StartingVcn;
	fullClustersCount = GetFullClustersCount(file);
	movFileStruct.StartingLcn = GetStartLcn(fullClustersCount, vbb);
	movFileStruct.ClusterCount = file.buffer->Extents[0].NextVcn.QuadPart - file.buffer->StartingVcn.QuadPart;
	wcout << drive.Letter;
	prevVcn = file.buffer->StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < file.buffer->ExtentCount; i++){
		movFileStruct.ClusterCount = file.buffer->Extents[i].NextVcn.QuadPart - prevVcn;
		bool ret = DeviceIoControl(drive.hDisk, FSCTL_MOVE_FILE, &movFileStruct, sizeof(MOVE_FILE_DATA), NULL, 0, &pBytes, NULL);
		if (!ret){
			if (GetLastError() == ERROR_ACCESS_DENIED){
				cout << "nu emae" << GetLastError();
				return false;
			}
			else{
				cout << "nu emae" << GetLastError();
				return false;
			}
		}
		else{
			cout << "yes";
			movFileStruct.StartingLcn.QuadPart += movFileStruct.ClusterCount;
			movFileStruct.StartingVcn.QuadPart += movFileStruct.ClusterCount;
			prevVcn = file.buffer->Extents[i].NextVcn.QuadPart;
				
		}
	}
	return true;
}