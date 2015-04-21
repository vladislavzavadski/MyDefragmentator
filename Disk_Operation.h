#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

bool InitializeHandle(Disk* drive){
	WCHAR diskName[30] = L"\\\\.\\";
	wcscat(diskName, drive->Letter);
	diskName[6] = L'\0';
	drive->hDisk = CreateFile(diskName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, FILE_READ_ATTRIBUTES, NULL);
	if (drive->hDisk == NULL || drive->hDisk == INVALID_HANDLE_VALUE){
		return false;
	}
	return true;
}

void ShowDiskInformation(Disk drive){
	PARTITION_INFORMATION_EX pdg;
	unsigned long long int qq;
	ULONGLONG DiskSize;
	DWORD cbWritten = 0;
	if (drive.hDisk == NULL || drive.hDisk == INVALID_HANDLE_VALUE){
		cout << "323";		
	}
	if (DeviceIoControl(drive.hDisk,
		IOCTL_DISK_GET_PARTITION_INFO_EX,
		NULL, 0,
		&pdg, sizeof(pdg),     
		&cbWritten,                 
		(LPOVERLAPPED)NULL))
	{

		printf("%lli bytes\n", pdg.PartitionLength);
		
	}
	else{
		wcout << "Can not show information about disk " << drive.Letter << endl<<GetLastError();
	}
}

VOLUME_BITMAP_BUFFER Get_Volume_BitMap(Disk drive){
	STARTING_LCN_INPUT_BUFFER InBuf;
	VOLUME_BITMAP_BUFFER OutBuf;
	PVOLUME_BITMAP_BUFFER pOutBuf = &OutBuf;
	_int64 numberOfClusters;
	DWORD pBytes;
	int ret;
	_int64 nOutSize;
	nOutSize = sizeof(VOLUME_BITMAP_BUFFER);
	InBuf.StartingLcn.QuadPart = 0;
	ret = DeviceIoControl(drive.hDisk, FSCTL_GET_VOLUME_BITMAP, &InBuf, sizeof(InBuf),
		pOutBuf, nOutSize, &pBytes, NULL);
	if (!ret&&GetLastError()==ERROR_MORE_DATA){
		numberOfClusters = pOutBuf->BitmapSize.QuadPart - pOutBuf->StartingLcn.QuadPart;
		nOutSize = numberOfClusters + sizeof(VOLUME_BITMAP_BUFFER);
		pOutBuf = (PVOLUME_BITMAP_BUFFER)malloc(nOutSize);
		pOutBuf->StartingLcn.QuadPart = 0;
		ret = DeviceIoControl(drive.hDisk, FSCTL_GET_VOLUME_BITMAP, &InBuf, sizeof(InBuf),
			pOutBuf, nOutSize, &pBytes, NULL);
		if (ret){
			printf("%lli\n", pOutBuf->StartingLcn);
			printf("%lli\n", pOutBuf->BitmapSize);
			return *pOutBuf;
			
		}
	}
	else{
		return *pOutBuf;
	}
}