#include <iostream>
#include "Interface.h"
#include <conio.h>
#include "CheckUser.h"
#include <stack>
#include <dos.h>
//#include "Sealed.h"
#define NO_SUITABLE_AREA -1
//#include "AnalizeFile.h"
using namespace std;

int main(){
	int diskCount;
	PVOLUME_BITMAP_BUFFER VBB;
	stack<wstring> st;
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
				DefragmentateFile(FI->hFile, FI->buffer, VBB, allDisks[choice2 - '0' - 1]);
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
			char timeStr[9];
			_strtime(timeStr);
			cout << "\nStarting disk analysis..."<<timeStr<< endl;
			AnalyzeDisk(allDisks[choice - '0' - 1]);
			_strtime(timeStr);
			cout << "End disk analysis..." << timeStr << endl;
			_strtime(timeStr);
			cout << "Starting file sealed..."<< timeStr << endl;
			SealedFilesOnDisk(allDisks[choice - '0' - 1], Get_Volume_BitMap(allDisks[choice-'0'-1]));
			_strtime(timeStr);
			cout << "End file sealed..." << timeStr << endl;
			_strtime(timeStr);
			cout << "Starting defragmentation " << timeStr << endl;
			if (!beginThread(allDisks[choice - '0' - 1])){
				cout << "Undefined Error" << endl;
				system("pause");
				return 0;
			}
			while (1){
			VBB = Get_Volume_BitMap(allDisks[choice - '0' - 1]);
			if (!VBB){
				cout << "Error with reading volume map code:  " << GetLastError() << endl;
				break;
			}
			path = nextFile();
			if (path == L"")
				break;
			wcout << "\nDefragment file: " << path.c_str() << endl;
			FI = checkFileClusters(path.c_str());
			if (!FI){
				cout << "Error with file opening" << endl;
				system("pause");
				break;
			}
			if (FI->buffer->ExtentCount != 1 && FI->buffer->ExtentCount != 0){
				if (NO_SUITABLE_AREA == DefragmentateFile(FI->hFile, FI->buffer, VBB, allDisks[choice - '0' - 1])){
					st.push(path);
				}
			}
			free(VBB);
			CloseHandle(FI->hFile);
			free(FI);
			}
			SealedFilesOnDisk(allDisks[choice - '0' - 1], Get_Volume_BitMap(allDisks[choice - '0' - 1]));
			if (!st.empty()){
				while (!st.empty()){
					wstring temp = st.top();
					FI = checkFileClusters(path.c_str());
					if (!FI){
						cout << "Error with file opening" << endl;
						system("pause");
						break;
					}
					if (FI->buffer->ExtentCount != 1 && FI->buffer->ExtentCount != 0){
						DefragmentateFile(FI->hFile, FI->buffer, VBB, allDisks[choice - '0' - 1]);
							
						
					}
				}
			}
			_strtime(timeStr);
			cout << "\nEnd defragmentation..." << timeStr << endl;
			_strtime(timeStr);
			cout << "\nStarting disk analysis..." << timeStr << endl;
			AnalyzeDisk(allDisks[choice - '0' - 1]);
			_strtime(timeStr);
			cout << "End disk analysis..." << timeStr << endl;
		}
		if(choice2 == '2'){
			readVolumeMap(allDisks[choice - '0' - 1]);
		}
		
	}

	system("pause");
	return 0;
}