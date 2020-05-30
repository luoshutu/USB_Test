// USB_Test.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "string"
#include "../DataTransfer/DataTransfer.h"

#ifdef  _DEBUG
#pragma comment(lib,"../x64/Debug/DataTransfer.lib")
#else
#pragma comment(lib,"../x64/Release/DataTransfer.lib")
#endif

using namespace std;

int main()
{	
	int usbDeviceNumber = 0;
	usbDeviceNumber = GetUSBDeviceCount();

	string serialNumber = " ";
	serialNumber = GetSerialNumber();

	cout << usbDeviceNumber << '\n';
	cout << serialNumber << '\n';
	system("pause");

    return 0;
}

