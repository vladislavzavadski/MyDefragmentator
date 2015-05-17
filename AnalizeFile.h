#include <Windows.h>
#include <iostream>
using namespace std;

typedef struct {
	HANDLE hFile;
	PRETRIEVAL_POINTERS_BUFFER buffer;
}FILE_INFO;

unsigned long long int StartLcn1;
unsigned long long int FullClustersCount1;
unsigned long long int dlinaOblasti;

unsigned long long int getStartLcn1(){
	return StartLcn1;
}

unsigned long long int getdlinaOblasti(){
	return dlinaOblasti;
}

unsigned long long int getFullClustersCount1(){
	return FullClustersCount1;
}

FILE_INFO* checkFileClusters(const wchar_t* pathToFile){
	FILE_INFO *temp;
	STARTING_VCN_INPUT_BUFFER SVIB;
	RETRIEVAL_POINTERS_BUFFER rpb;
	PRETRIEVAL_POINTERS_BUFFER RPB = &rpb;
	_int64 pOutSize = 0;
	int errorCode;
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
	unsigned long long int i = RPB->Extents[1].NextVcn.QuadPart;
		temp->buffer = RPB;
		i = temp->buffer->Extents[1].NextVcn.QuadPart;
	return temp;
}


LARGE_INTEGER GetStartLcn(unsigned long long int clustersLength, PVOLUME_BITMAP_BUFFER vbb){
	LARGE_INTEGER StartLcn, EndLcn;
	unsigned long long int count = 0;
	unsigned long long int prevCount = 0;
	unsigned long long int startMaxLcn = 0;
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
			if (count > prevCount){
				startMaxLcn = StartLcn.QuadPart;
				prevCount = count;
			}
			count = 0;
		}

		if (count == clustersLength){
			EndLcn.QuadPart = i;
			break;
		}

	}
	if (count > prevCount){
		prevCount = count;
		StartLcn1 = StartLcn.QuadPart;

	}
	else{
		StartLcn1 = startMaxLcn;
	}
	if (count != clustersLength){
		StartLcn.QuadPart = -1;		
		FullClustersCount1 = clustersLength;
		dlinaOblasti = prevCount;
		return StartLcn;
	}
	StartLcn.QuadPart *= 8;
	return StartLcn;
}

unsigned long long int GetFullClustersCount(PRETRIEVAL_POINTERS_BUFFER file){
	unsigned long long int fullClustersCount = file->Extents[0].NextVcn.QuadPart - file->StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < file->ExtentCount-1; i++){
		fullClustersCount += -file->Extents[i].NextVcn.QuadPart + file->Extents[i + 1].NextVcn.QuadPart;
	}
	return fullClustersCount;
}


int DefragmentateFile(HANDLE hFile, PRETRIEVAL_POINTERS_BUFFER buffer, PVOLUME_BITMAP_BUFFER vbb, Disk drive){
	MOVE_FILE_DATA movFileStruct;
	unsigned long long int prevVcn;
	unsigned long long int fullClustersCount;
	LARGE_INTEGER lcn;
	DWORD pBytes = 0;

	movFileStruct.FileHandle = hFile;
	movFileStruct.StartingVcn = buffer->StartingVcn;
	unsigned long long int i = buffer->Extents[1].NextVcn.QuadPart;
	fullClustersCount = GetFullClustersCount(buffer);
	movFileStruct.StartingLcn = GetStartLcn(fullClustersCount, vbb);
	//movFileStruct.StartingLcn.QuadPart = 3908344;

	/*unsigned long long int l=vbb->Buffer[206072];
	l = vbb->Buffer[25759];
	for (unsigned long long int i = 7396; i < 56358; i++){
		if (vbb->Buffer[i] != 0){
			cout << endl << vbb->Buffer[i];
		}
	}*/
	if (movFileStruct.StartingLcn.QuadPart == -1){

		return -1;
	}
	movFileStruct.ClusterCount = buffer->Extents[0].NextVcn.QuadPart - buffer->StartingVcn.QuadPart;
	wcout << drive.Letter;
	prevVcn = buffer->StartingVcn.QuadPart;
	for (unsigned long long int i = 0; i < buffer->ExtentCount; i++){
		movFileStruct.ClusterCount = buffer->Extents[i].NextVcn.QuadPart - prevVcn;
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
			prevVcn = buffer->Extents[i].NextVcn.QuadPart;
				
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