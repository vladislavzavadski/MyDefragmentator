#include <iostream>
#include "Interface.h"
#include <conio.h>
#include "CheckUser.h"
#include "AnalizeFile.h"
using namespace std;

int main(){
	int diskCount;
	PVOLUME_BITMAP_BUFFER VBB;
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
		//Get_Volume_BitMap(allDisks[1]);
		wcout << diskCount + 1 << ". " << "Defragmentate File" << endl;
		wcout << diskCount + 2 << ". " << "Refresh" << endl;
		wcout << diskCount + 3 << ". " << "Exit" << endl;
		choice = _getch();

		if (choice == diskCount + 1 + '0'){
			system("CLS");
			wcout << "Select the drive where the file resides\n";
			diskCount = displayMenu(allDisks);
			choice2 = _getch();
			if (choice2 - '0' > diskCount||choice2<'1'){
				wcout << "Error of choice" << endl;
				system("pause");
				continue;
			}
			else{
				VBB = Get_Volume_BitMap(allDisks[choice2 - '0' - 1]);
				wcout << "Enter Full Path to File" << endl;
				FI = Get_RETRIEVAL_POINTERS_BUFFER_Of_File(L"C:\\Users\\Владислав\\Downloads\\jdk-8u31-windows-x64.exe");
				if (!FI){
					cout << "Error with file opening" << endl;
					continue;
				}
				DefragmentateFile(*FI, VBB, allDisks[choice2 - '0' - 1]);
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
		
	}

	system("pause");
	return 0;
}