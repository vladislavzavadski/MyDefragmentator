#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

bool InitializeHandle(Disk* drive){
	WCHAR diskName[30] = L"\\\\.\\";
	wcscat(diskName, drive->Letter);
	diskName[6] = L'\0';
	drive->hDisk = CreateFile(diskName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,OPEN_EXISTING, 0, NULL);
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

unsigned long long int GetBusyClasters(BYTE ik){
	unsigned long long int count = 0;
	int ost;
	for (int i = 0; i < 8; i++){
		ost = ik % 2;
		if (ost==1)
			count++;
		ik /= 2;
	}
	return count;
}

unsigned long long int GetFreeClasters(BYTE ik){
	unsigned long long int count = 0;
	int ost;
	for (int i = 0; i < 8; i++){
		ost = ik % 2;
		if (ost == 0){
			count++;
		}
		ik /= 2;
	}
	return count;
}

void showVolumeMap(PNTFS_VOLUME_DATA_BUFFER info){
	system("CLS");
	printf("1. Volume Serial Number %lli\n", info->VolumeSerialNumber);
	printf("2. Number of sectors %lli\n", info->NumberSectors);
	printf("3. Total number of clusters %lli\n", info->TotalClusters);
	printf("4. Number of free clusters %lli\n", info->FreeClusters);
	printf("5. Number of reserved clusters %lli\n", info->TotalReserved);
	printf("6. Number of bytes in sector %d\n", info->BytesPerSector);
	printf("7. Number of bytes in cluster %d\n", info->BytesPerCluster);
	printf("8. Number of bytes in a file record segment %d\n", info->BytesPerFileRecordSegment);
	printf("9. Number of clusters in a file record segment %d\n", info->ClustersPerFileRecordSegment);
	printf("10. Length of master file table %lli bytes\n", info->MftValidDataLength);
	printf("11. The starting logical cluster number of the master file table %lli\n", info->MftStartLcn);
	printf("12. The starting logical cluster number of the master file table mirror. %lli\n", info->Mft2StartLcn);
	printf("13. The starting logical cluster number of the master file table zone. %lli\n", info->MftZoneEnd);
	printf("14. The ending logical cluster number of the master file table zone. %lli\n", info->MftZoneEnd);
	system("pause");
}

void readVolumeMap(Disk drive){
	PNTFS_VOLUME_DATA_BUFFER info;
	DWORD cbWritten;
	info = (PNTFS_VOLUME_DATA_BUFFER)malloc(sizeof(NTFS_VOLUME_DATA_BUFFER));
	if (!DeviceIoControl(drive.hDisk, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0,
		info, sizeof(NTFS_VOLUME_DATA_BUFFER), &cbWritten, NULL)){
		system("CLS");
		cout << "Error Number " << GetLastError() << endl;
		system("pause");
		return;
	}
	else{
		showVolumeMap(info);
	}
}

PVOLUME_BITMAP_BUFFER Get_Volume_BitMap(Disk drive){
	STARTING_LCN_INPUT_BUFFER InBuf;
	VOLUME_BITMAP_BUFFER OutBuf;
	PVOLUME_BITMAP_BUFFER pOutBuf = &OutBuf;
	NTFS_VOLUME_DATA_BUFFER NtfsData;
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
			return pOutBuf;
		}

	}
	else{
		if (!ret){
			return NULL;
		}
		return pOutBuf;
	}

}