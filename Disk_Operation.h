﻿#include <windows.h>
#include <string>
#include <iostream>
#include <cvt\wstring>
#include <string.h>
#include <time.h>
#include "AnalizeFile.h"
#define BUFFER_SIZE (1024*1024)
#define SIZE 64
using namespace std;
using namespace stdext;
using namespace cvt;
USN maxusn;
int count11 = 0;
bool threadWorking;
wstring pathToFile;
wstring slash = L"\\";
wstring pathToProcess;
wstring letter;
HANDLE hFile;
HANDLE hDisk;
HANDLE thread;
DWORD input_Structure;
HANDLE hEvent1, hEvent2;
DWORD WINAPI processFile(LPVOID point){
 	FILE_INFO *fi;
	fi = checkFileClusters(pathToProcess.c_str());
	if (!fi){
		count11++;
	}
	return 0;
}

bool IsDirectory(DWORD FileAttrs)
{
	return (
		(0 != (FileAttrs & FILE_ATTRIBUTE_DIRECTORY)) &&
		(0 == (FileAttrs & FILE_ATTRIBUTE_REPARSE_POINT))
		);
}

bool InitializeHandle(Disk* drive){
	WCHAR diskName[30] = L"\\\\.\\";
	wcscat(diskName, drive->Letter);
	diskName[6] = L'\0';
	drive->hDisk = CreateFile(diskName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (drive->hDisk == NULL || drive->hDisk == INVALID_HANDLE_VALUE){
		cout << "Error: " << GetLastError();
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
	LOOKUP_STREAM_FROM_CLUSTER_INPUT inpStruct;
	LOOKUP_STREAM_FROM_CLUSTER_ENTRY str;
	LOOKUP_STREAM_FROM_CLUSTER_OUTPUT str1;
	PLARGE_INTEGER INT = (PLARGE_INTEGER)malloc(sizeof(LARGE_INTEGER));
	DWORD cbWritten;
	int size = (sizeof(DWORD) * 2 + sizeof(LARGE_INTEGER));
	inpStruct.NumberOfClusters = 1;
	inpStruct.Cluster[0].QuadPart = 26585528;

	info = (PNTFS_VOLUME_DATA_BUFFER)malloc(sizeof(NTFS_VOLUME_DATA_BUFFER));
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

void record_file_name(USN_RECORD *record){
	void * buffer;
	int size = 128;
	MFT_ENUM_DATA mft_enum_data;
	DWORD bytecount = 1;
	USN_RECORD * parent_record;
	bool flag = false;
	WCHAR * filename;
	WCHAR * filenameend;
	wstring filenameTemp;
	wstring filenameendTemp;
	filename = (WCHAR *)(((BYTE *)record) + record->FileNameOffset);
	filenameend = (WCHAR *)(((BYTE *)record) + record->FileNameOffset + record->FileNameLength);
	filenameTemp = filename;
	filenameendTemp = filenameend;
	if (wcsncmp(filenameend, L"", 1) != 0){
		filenameTemp[filenameTemp.size() - filenameendTemp.size()] = L'\0';
	}

	if (pathToFile.size()){
		pathToFile = filenameTemp + slash + pathToFile;
	}
	else{
		pathToFile = filenameTemp;
	}
	buffer = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (buffer == NULL)
	{
		return;
	}

	mft_enum_data.StartFileReferenceNumber = record->ParentFileReferenceNumber;
	mft_enum_data.LowUsn = 0;
	mft_enum_data.HighUsn = maxusn;
	repeat:
	bool ret = DeviceIoControl(hDisk, FSCTL_ENUM_USN_DATA, &mft_enum_data, sizeof(mft_enum_data), buffer, size, &bytecount, NULL);
	if (!ret){
		cout << "ewr";
	}
	while (!ret&&GetLastError() == ERROR_MORE_DATA)
	{
		VirtualFree(buffer, 0, MEM_RELEASE);
		size *= 2;
		buffer = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (!buffer){
			return;
		}
		ret = DeviceIoControl(hDisk, FSCTL_ENUM_USN_DATA, &mft_enum_data, sizeof(mft_enum_data), buffer, size, &bytecount, NULL);
		
	}
	if (!ret&&GetLastError() != ERROR_MORE_DATA){
		VirtualFree(buffer, 0, MEM_RELEASE);
		return;
	}

	parent_record = (USN_RECORD *)((USN *)buffer + 1);
	if (parent_record->RecordLength == 0&&!flag){
		size *= 8;
		VirtualFree(buffer, 0, MEM_RELEASE);
		buffer = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		goto repeat;
	}
	if (parent_record->FileReferenceNumber != record->ParentFileReferenceNumber)
	{
		VirtualFree(buffer, 0, MEM_RELEASE);
		return;
	}

	record_file_name(parent_record);
	VirtualFree(buffer, 0, MEM_RELEASE);
}


DWORD WINAPI doSomething(LPVOID point){
	MFT_ENUM_DATA mft_enum_data;
	pathToFile = L"";
	DWORD bytecount = 1;
	void * buffer;
	USN_RECORD * record;
	USN_RECORD * recordend;
	USN_JOURNAL_DATA * journal;
	DWORDLONG nextid;
	DWORDLONG filecount = 0;
	DWORD starttick, endtick, inputStruct;
	HANDLE hThread;
	bool flag = 0;
	starttick = GetTickCount();


	buffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (buffer == NULL)
	{

		return 0;
	}


	if (!DeviceIoControl(hDisk, FSCTL_QUERY_USN_JOURNAL, NULL, 0, buffer, BUFFER_SIZE, &bytecount, NULL))
	{
		if (GetLastError() == 1179){
			CREATE_USN_JOURNAL_DATA inpStr;
			DWORD cbWritten;
			inpStr.MaximumSize = 1073741824;
			inpStr.AllocationDelta = 1048576;
			if (!DeviceIoControl(hDisk, FSCTL_CREATE_USN_JOURNAL, &inpStr, sizeof(CREATE_USN_JOURNAL_DATA), NULL, 0, &cbWritten, NULL)){
				cout << "Error: " << GetLastError();
				return 0;
			}
		}
	}

	journal = (USN_JOURNAL_DATA *)buffer;

	maxusn = journal->MaxUsn;

	mft_enum_data.StartFileReferenceNumber = 0;
	mft_enum_data.LowUsn = 0;
	mft_enum_data.HighUsn = maxusn;
	clock_t start = clock(), end;
	for (;;)
	{


		if (!DeviceIoControl(hDisk, FSCTL_ENUM_USN_DATA, &mft_enum_data, sizeof(mft_enum_data), buffer, BUFFER_SIZE, &bytecount, NULL))
		{
			end = clock();

			pathToFile = letter + pathToFile;
			threadWorking = false;
			SetEvent(hEvent1);
			return 0;
		}


		nextid = *((DWORDLONG *)buffer);

		record = (USN_RECORD *)((USN *)buffer + 1);
		recordend = (USN_RECORD *)(((BYTE *)buffer) + bytecount);

		while (record < recordend)
		{
			filecount++;


			if (record->FileName[0] != L'$'&&!IsDirectory(record->FileAttributes)){
				record_file_name(record);
				pathToFile = letter + pathToFile;
				pathToProcess = pathToFile;
				SetEvent(hEvent1);
				WaitForSingleObject(hEvent2, INFINITE);
			}
			pathToFile.clear();
			record = (USN_RECORD *)(((BYTE *)record) + record->RecordLength);
		}

		mft_enum_data.StartFileReferenceNumber = nextid;
	}
	return 0;
}

wstring nextFile(){
	wstring temp;
	int i = 0;
	WaitForSingleObject(hEvent1, INFINITE);
	if (!threadWorking){
		CloseHandle(hEvent1);
		CloseHandle(hEvent2);
		CloseHandle(thread);
		return L"";
	}
	temp = pathToProcess;
	
	SetEvent(hEvent2);
	return temp;
}

bool beginThread(Disk drive){
	hDisk = drive.hDisk;
	letter = drive.Letter;
	hEvent1 = CreateEvent(NULL, false, false, L"Event1");
	if (hEvent1 == NULL || hEvent1 == INVALID_HANDLE_VALUE){
		return false;
	}
	hEvent2 = CreateEvent(NULL, false, false, L"Event2");
	if (hEvent2 == NULL || hEvent2 == INVALID_HANDLE_VALUE){
		return false;
	}
	thread = CreateThread(NULL, 0, doSomething, NULL, 0, &input_Structure);
	if (thread == NULL || thread == INVALID_HANDLE_VALUE){
		return false;
	}
	threadWorking = true;
	return true;

}
void AnalyzeDisk(Disk drive){
	wstring pathToFile;
	FILE_INFO *FI;
	string name;
	cout << "File Analize: " << endl;
	beginThread(drive);
	while (1){
		pathToFile = nextFile();
		if (pathToFile == L""){
			return;
		}
		string name(pathToFile.begin(), pathToFile.end());
		cout << "File: " << name << " Extents: " ;
		FI = checkFileClusters(pathToFile.c_str());
		cout << FI->buffer->ExtentCount << endl;
		free(FI);
	}
	return;
}

unsigned long long int searchFreeSpaceBefore(PVOLUME_BITMAP_BUFFER vbb, unsigned long long int lcn){
	unsigned long long int freeSpaceLength = 0;
	unsigned long long j = lcn - 1;
	unsigned long long int limit = vbb->BitmapSize.QuadPart*0.2;
	while (vbb->Buffer[j] == 0&&j!=0){
		freeSpaceLength++;
		j--;
	}
	return freeSpaceLength;
}

int moveFileCluster(unsigned long long int start, HANDLE hFile, HANDLE hDisk, unsigned long long int vcn){
	MOVE_FILE_DATA movData;
	DWORD cbWritten;
	movData.ClusterCount = 1;
	movData.FileHandle = hFile;
	movData.StartingLcn.QuadPart = start;
	movData.StartingVcn.QuadPart = vcn;
	bool result = DeviceIoControl(hDisk, FSCTL_MOVE_FILE, &movData, sizeof(movData), NULL, 0, &cbWritten, NULL);
	if (!result){
		return 5;
	}
	else
		return 0;
	
}


void leftShift(FILE_INFO *fi, PVOLUME_BITMAP_BUFFER VBB, Disk drive){
	unsigned long long int extentLength;
	unsigned long long int prevVcn = fi->buffer->StartingVcn.QuadPart;
	unsigned long long int lcn;
	unsigned long long int count = 0;
	unsigned long long int freeSpaceLength;
	int offset;
	unsigned long long int vcnNumber = fi->buffer->StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < fi->buffer->ExtentCount; i++){
		metka:
		lcn = fi->buffer->Extents[i].Lcn.QuadPart;
		extentLength = fi->buffer->Extents[i].NextVcn.QuadPart - prevVcn;
		prevVcn = fi->buffer->Extents[i].NextVcn.QuadPart;
		count = 0;
		if (freeSpaceLength = searchFreeSpaceBefore(VBB, lcn)){
			while (extentLength!=0){
				while (moveFileCluster(((lcn - freeSpaceLength + count)), fi->hFile, drive.hDisk, vcnNumber)==5){
					count++;
					if (count == freeSpaceLength){
						i++;
						goto metka;
					}
				}
				extentLength--;
				vcnNumber++;
				count++;
			}	
		}
		vcnNumber = fi->buffer->Extents[i].NextVcn.QuadPart;
		free(VBB);
		VBB = Get_Volume_BitMap(drive);
	}
}

void leftShift(HANDLE hFile, RETRIEVAL_POINTERS_BUFFER buffer, PVOLUME_BITMAP_BUFFER VBB, Disk drive){
	unsigned long long int extentLength;
	unsigned long long int prevVcn = buffer.StartingVcn.QuadPart;
	unsigned long long int lcn;
	unsigned long long int count = 0;
	unsigned long long int freeSpaceLength;
	int offset;
	unsigned long long int vcnNumber = buffer.StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < buffer.ExtentCount; i++){
	metka:
		lcn = buffer.Extents[i].Lcn.QuadPart;
		extentLength = buffer.Extents[i].NextVcn.QuadPart - prevVcn;
		prevVcn = buffer.Extents[i].NextVcn.QuadPart;
		count = 0;
		if (freeSpaceLength = searchFreeSpaceBefore(VBB, lcn)){
			while (extentLength != 0){
				while (moveFileCluster(((lcn - freeSpaceLength + count)), hFile, drive.hDisk, vcnNumber) == 5){
					count++;
					if (count == freeSpaceLength){
						i++;
						goto metka;
					}
				}
				extentLength--;
				vcnNumber++;
				count++;
			}
		}
		vcnNumber = buffer.Extents[i].NextVcn.QuadPart;
		free(VBB);
		VBB = Get_Volume_BitMap(drive);
	}
}

void SealedFilesOnDisk(Disk drive, PVOLUME_BITMAP_BUFFER VBB){
	wstring pathToFile;
	MOVE_FILE_DATA ff;
	FILE_INFO *FI = NULL;
	beginThread(drive);
	
	while (1){
		pathToFile = nextFile();
		if (pathToFile == L""){
			break;
		}
		free(FI);
		FI = checkFileClusters(pathToFile.c_str());
		if (FI->buffer->ExtentCount == 1){
			leftShift(FI->hFile, *FI->buffer, VBB, drive);
		}
		else{
			leftShift(FI, VBB, drive);
		}
		free(VBB);
		VBB = Get_Volume_BitMap(drive);
	}
}

