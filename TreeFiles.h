#include <Windows.h>
#include <conio.h>
#include <iostream>
#include "Disk.h"
#include <stdio.h>
#include <string>
#define BUFFER_SIZE (1024*1024)
using namespace std;

USN maxusn;
wstring pathToFile;

void record_file_name(USN_RECORD *record, Disk drive){
	void * buffer;
	MFT_ENUM_DATA mft_enum_data;
	DWORD bytecount = 1;
	USN_RECORD * parent_record;

	WCHAR * filename;
	WCHAR * filenameend;

	filename = (WCHAR *)(((BYTE *)record) + record->FileNameOffset);
	filenameend = (WCHAR *)(((BYTE *)record) + record->FileNameOffset + record->FileNameLength);

	pathToFile = filename + pathToFile;

	buffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (buffer == NULL)
	{
		return;
	}

	mft_enum_data.StartFileReferenceNumber = record->ParentFileReferenceNumber;
	mft_enum_data.LowUsn = 0;
	mft_enum_data.HighUsn = maxusn;

	if (!DeviceIoControl(drive.hDisk, FSCTL_ENUM_USN_DATA, &mft_enum_data, sizeof(mft_enum_data), buffer, BUFFER_SIZE, &bytecount, NULL))
	{
		return;
	}

	parent_record = (USN_RECORD *)((USN *)buffer + 1);

	if (parent_record->FileReferenceNumber != record->ParentFileReferenceNumber)
	{

		return;
	}

	record_file_name(parent_record, drive);
}

bool doSomething(Disk drive){
	MFT_ENUM_DATA mft_enum_data;
//	pathToFile = drive.Letter;
	pathToFile = L"";
	DWORD bytecount = 1;
	void * buffer;
	USN_RECORD * record;
	USN_RECORD * recordend;
	USN_JOURNAL_DATA * journal;
	DWORDLONG nextid;
	DWORDLONG filecount = 0;
	DWORD starttick, endtick;

	starttick = GetTickCount();


	buffer = VirtualAlloc(NULL, BUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (buffer == NULL)
	{
		return 0;
	}


	if (!DeviceIoControl(drive.hDisk, FSCTL_QUERY_USN_JOURNAL, NULL, 0, buffer, BUFFER_SIZE, &bytecount, NULL))
	{
		if (GetLastError() == 1179){
			CREATE_USN_JOURNAL_DATA inpStr;
			DWORD cbWritten;
			inpStr.MaximumSize = 1073741824;
			inpStr.AllocationDelta = 1048576;
			if (!DeviceIoControl(drive.hDisk, FSCTL_CREATE_USN_JOURNAL, &inpStr, sizeof(CREATE_USN_JOURNAL_DATA), NULL, 0, &cbWritten, NULL)){
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

	for (;;)
	{


		if (!DeviceIoControl(drive.hDisk, FSCTL_ENUM_USN_DATA, &mft_enum_data, sizeof(mft_enum_data), buffer, BUFFER_SIZE, &bytecount, NULL))
		{
			pathToFile = drive.Letter + pathToFile;
			return 0;
		}


		nextid = *((DWORDLONG *)buffer);

		record = (USN_RECORD *)((USN *)buffer + 1);
		recordend = (USN_RECORD *)(((BYTE *)buffer) + bytecount);

		while (record < recordend)
		{
			filecount++;
			record_file_name(record, drive);
			record = (USN_RECORD *)(((BYTE *)record) + record->RecordLength);
		}

		mft_enum_data.StartFileReferenceNumber = nextid;
	}
	_getch();
	return 0;
}