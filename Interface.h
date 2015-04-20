#include <iostream>
#include <string>
#include "Disk.h"
#include "Disk_Operation.h"
using namespace std;

int displayMenu(Disk* allDisks){
	HANDLE hFind;
	int i = 0;
	Disk temp;
	DWORD write;
	cout << "Choose:  " << endl;
	hFind = FindFirstVolume(temp.individualName, 1024);
	GetVolumePathNamesForVolumeName(temp.individualName, temp.Letter, sizeof(temp.individualName), &write);
	wcscpy(allDisks[i].individualName, temp.individualName);
	wcscpy(allDisks[i].Letter, temp.Letter);
	InitializeHandle(&allDisks[i]);
	i++;
	while (FindNextVolume(hFind, temp.individualName, 1024)){
		GetVolumePathNamesForVolumeName(temp.individualName, temp.Letter, sizeof(temp.individualName), &write);
		wcscpy(allDisks[i].individualName, temp.individualName);
		wcscpy(allDisks[i].Letter, temp.Letter);
		InitializeHandle(&allDisks[i]);
		i++;
	}
	for (int j = 0; j < i; j++){
		wcout << j + 1 << ". " << allDisks[j].Letter << endl;
	}

	return i;
}