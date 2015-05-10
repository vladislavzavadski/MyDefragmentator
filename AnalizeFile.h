#include <Windows.h>
#include <iostream>
using namespace std;

typedef struct {
	HANDLE hFile;
	PRETRIEVAL_POINTERS_BUFFER buffer;
}FILE_INFO;


FILE_INFO* checkFileClusters(const wchar_t* pathToFile){
	FILE_INFO *temp;
	STARTING_VCN_INPUT_BUFFER SVIB;
	RETRIEVAL_POINTERS_BUFFER rpb;
	PRETRIEVAL_POINTERS_BUFFER RPB = &rpb;
	_int64 pOutSize = 0;
	DWORD bytes;
	temp = (FILE_INFO*)malloc(sizeof(FILE_INFO));
	temp->hFile = CreateFile(pathToFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (temp->hFile == INVALID_HANDLE_VALUE){
		cout << endl << GetLastError();
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

unsigned long long int GetFullClustersCount(PRETRIEVAL_POINTERS_BUFFER file){
	unsigned long long int fullClustersCount = file->Extents[0].NextVcn.QuadPart - file->StartingVcn.QuadPart;
	for (unsigned long long int i = 1; i < file->ExtentCount; i++){
		fullClustersCount += file->Extents[i].NextVcn.QuadPart - file->Extents[i - 1].NextVcn.QuadPart;
	}
	return fullClustersCount;
}

bool DefragmentateFile(HANDLE hFile, RETRIEVAL_POINTERS_BUFFER buffer, PVOLUME_BITMAP_BUFFER vbb, Disk drive){
	MOVE_FILE_DATA movFileStruct;
	unsigned long long int prevVcn;
	unsigned long long int fullClustersCount;
	LARGE_INTEGER lcn;
	DWORD pBytes = 0;
	if (buffer.ExtentCount == 1||buffer.ExtentCount==0){
		return true;
	}
	movFileStruct.FileHandle = hFile;
	movFileStruct.StartingVcn = buffer.StartingVcn;
	fullClustersCount = GetFullClustersCount(&buffer);
	movFileStruct.StartingLcn = GetStartLcn(fullClustersCount, vbb);
	movFileStruct.ClusterCount = buffer.Extents[0].NextVcn.QuadPart - buffer.StartingVcn.QuadPart;
	wcout << drive.Letter;
	prevVcn = buffer.StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < buffer.ExtentCount; i++){
		movFileStruct.ClusterCount = buffer.Extents[i].NextVcn.QuadPart - prevVcn;
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
			prevVcn = buffer.Extents[i].NextVcn.QuadPart;
				
		}
	}
	return true;
}

bool searchFileByItCluster(Disk drive){
	LOOKUP_STREAM_FROM_CLUSTER_INPUT inpStruct;
	LOOKUP_STREAM_FROM_CLUSTER_ENTRY str;
	LOOKUP_STREAM_FROM_CLUSTER_OUTPUT str1;
	PNTFS_VOLUME_DATA_BUFFER info;
	PLARGE_INTEGER INT = (PLARGE_INTEGER)malloc(sizeof(LARGE_INTEGER));
	DWORD cbWritten;
	int size = (sizeof(DWORD) * 2 + sizeof(LARGE_INTEGER));
	inpStruct.NumberOfClusters = 1;
	inpStruct.Cluster[0].QuadPart = 26585528;
	info = (PNTFS_VOLUME_DATA_BUFFER)malloc(sizeof(NTFS_VOLUME_DATA_BUFFER));
	bool ret = DeviceIoControl(drive.hDisk, FSCTL_LOOKUP_STREAM_FROM_CLUSTER, &inpStruct, (sizeof(DWORD) * 2 + sizeof(LARGE_INTEGER)), &str1, sizeof(LOOKUP_STREAM_FROM_CLUSTER_OUTPUT), &cbWritten, NULL);
	if (!ret){
		cout <<"Error: "<< GetLastError()<<endl<<sizeof(LOOKUP_STREAM_FROM_CLUSTER_INPUT);
		for (int i = 0; i < inpStruct.NumberOfClusters; i++){
			printf("%lli\n", inpStruct.Cluster[i]);
		}
		

		int i = 0;
	}
	return false;
}

bool enumUSNData(Disk drive){
	MFT_ENUM_DATA_V0 inpStruct;
	USN_RECORD_V2 outpStruct;
	DWORD cbWritten;
	inpStruct.StartFileReferenceNumber = 0;
	inpStruct.LowUsn = 100;
	inpStruct.HighUsn = 2000;
	if (!DeviceIoControl(drive.hDisk, FSCTL_ENUM_USN_DATA, &inpStruct, sizeof(MFT_ENUM_DATA_V0), &outpStruct, sizeof(USN_RECORD_V2), &cbWritten, NULL)){
		cout << "Error: " << GetLastError() << endl;
		system("pause");
		return false;
	}
	return true;

}