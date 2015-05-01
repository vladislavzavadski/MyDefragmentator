#include <iostream>
#include <string>
#include <Windows.h>
using namespace std;

class stack{
public:
	wstring dirName;
	HANDLE hFind;
	stack *next;
	stack *head;
public:
	stack(){
		this->head = NULL;
	}
	stack(wstring pi, HANDLE hFind){
		this->dirName = pi;
		this->hFind = hFind;
		this->head = NULL;
		this->next = NULL;
	}
	void push(stack* newProcessInfo){
		if (!head){
			head = newProcessInfo;
			head->next = NULL;
		}
		else{
			newProcessInfo->next = head;
			head = newProcessInfo;
		}
	}
	stack* pop(){
		stack* temp = head;
		stack* returnedValue = new stack(head->dirName, head->hFind);
		head = head->next;
		delete temp;
		return returnedValue;
	}


};