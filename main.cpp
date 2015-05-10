#include <iostream>
#include "Interface.h"
#include <conio.h>
#include "CheckUser.h"
//#include "AnalizeFile.h"
using namespace std;

int main(){
	int diskCount;
	PVOLUME_BITMAP_BUFFER VBB;
	wstring path;
	DWORD bye;
	FILE_INFO *FI;
	char choice2;
	WCHAR pathToFile[1024];
	HANDLE hFile;
	char choice;
	Disk *allDisks;
	allDisks = new Disk[6];
	if (!IsUserAdmin()){
		cout << "Sorry but you are not an administrator..."<<endl<<"The program will be closed...";
		_getch();
		return 0;
	}
	while (1){
		diskCount = displayMenu(allDisks);
//		searchFileByItCluster(allDisks[1]);
		//doSomething(allDisks[1]);
		wcout << diskCount + 1 << ". " << "Defragmentate File" << endl;
		wcout << diskCount + 2 << ". " << "Refresh" << endl;
		wcout << diskCount + 3 << ". " << "Exit" << endl;
		fflush(stdin);
		choice = _getch();

		if (choice == diskCount + 1 + '0'){
			system("CLS");
			wcout << "Select the drive where the file resides\n";
			diskCount = displayMenu(allDisks);
			fflush(stdin);
			choice2 = _getch();
			if (choice2 - '0' > diskCount||choice2<'1'){
				wcout << "Error of choice" << endl;
				system("pause");
				continue;
			}
			else{
				VBB = Get_Volume_BitMap(allDisks[choice2 - '0' - 1]);
				wcout << "Enter Full Path to File" << endl;
				FI = checkFileClusters(L"C:\\MainQueueOnline1.que");
				if (!FI){
					cout << "Error with file opening" << endl;
					continue;
				}
				DefragmentateFile(FI->hFile, *FI->buffer, VBB, allDisks[choice2 - '0' - 1]);
				continue;
			}
			wcout << "Enter full path to file: ";
			wcin >> pathToFile;
			continue;
		}
		if (choice == diskCount + 2 + '0'){
			system("CLS");
			continue;
		}
		if (choice == diskCount + 3 + '0'){
			return 0;
		}
		if (choice > diskCount + 3 + '0'){
			system("CLS");
			continue;
		}
		if (choice < '1'){
			system("CLS");
			continue;
		}
		system("CLS");
		printf("1. Defragment\n2. Show Info");
		fflush(stdin);
		choice2 = _getch();
		if (choice2 == '1'){
			if (!beginThread(allDisks[choice - '0' - 1])){
				cout << "Undefined Error" << endl;
				system("pause");
				return 0;
			}
			while (1){
			VBB = Get_Volume_BitMap(allDisks[choice - '0' - 1]);
			path = nextFile();
			if (path == L"")
				break;
			FI = checkFileClusters(path.c_str());
			if (!FI){
				cout << "Error with file opening" << endl;
				system("pause");
				break;
			}
			DefragmentateFile(FI->hFile, *FI->buffer, VBB, allDisks[choice2 - '0' - 1]);
			free(VBB);
			free(FI);
			}
		}
		if(choice2 == '2'){
			readVolumeMap(allDisks[choice - '0' - 1]);
			//enumUSNData(allDisks[1]);
		}
		
	}

	system("pause");
	return 0;
}