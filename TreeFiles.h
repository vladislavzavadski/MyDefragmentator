#include <iostream>
#include <string>
#include <Windows.h>
#include "stack.h"
using namespace std;

bool IsNotDotName(std::wstring const & Filename)
{
	return ((Filename != L".") && (Filename != L".."));
}



bool IsDirectory(DWORD FileAttrs)
{
	return (
		(0 != (FileAttrs & FILE_ATTRIBUTE_DIRECTORY)) &&
		(0 == (FileAttrs & FILE_ATTRIBUTE_REPARSE_POINT))
		);
}

void goOnFilesTree(wstring sourceStr){
	WIN32_FIND_DATA data;
	stack dirStack;
	stack *temp;
	bool flag = true;
	HANDLE hFind;
	while (1){
		if (flag == true){
			hFind = FindFirstFile((sourceStr + L"*").c_str(), &data);
			flag = false;
		}

		if (IsDirectory(data.dwFileAttributes) && IsNotDotName(&data.cFileName[0])){
			temp = new stack();
			temp->hFind = hFind;
			temp->dirName = sourceStr;
			dirStack.push(temp);
			sourceStr = sourceStr + data.cFileName + L"\\";
			flag = true;
			//delete temp;
			continue;
		}
		if (!IsDirectory(data.dwFileAttributes) && IsNotDotName(&data.cFileName[0])){
			wcout << sourceStr << data.cFileName << endl;
		}
	next:
		if (!FindNextFile(hFind, &data)){
			if (!dirStack.head){
				//				CloseHandle(hFind);
				break;
			}
			else{
				temp = dirStack.pop();
				//				CloseHandle(hFind);
				hFind = temp->hFind;
				sourceStr = temp->dirName;
				goto next;
			}
		}
	}


}