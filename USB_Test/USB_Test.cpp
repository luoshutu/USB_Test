// USB_Test.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "../DataTransfer/DataTransfer.h"

#ifdef  _DEBUG
#pragma comment(lib,"../x64/Debug/DataTransfer.lib")
#else
#pragma comment(lib,"../x64/Release/DataTransfer.lib")
#endif

using namespace std;

int main()
{	
	BOOL usbIsOpen = FALSE;
	BOOL usbStatue = FALSE;
	int usbNumber = 0;
	TCHAR usbDeviceInfo = 0;
	TCHAR usbVersion = 0;

	usbIsOpen = OpenUSBDevice();
	usbStatue = USBDeviceStatus();
	usbNumber = USBDeviceCount();
	usbDeviceInfo = GetDeviceInfo();
	usbVersion = GetUSBVersion();

	cout << usbIsOpen << '\n' << usbNumber << '\n';
	cout << usbDeviceInfo << '\n' << usbVersion << '\n';

	printf("Hello World!\n");
	system("pause");

    return 0;
}

