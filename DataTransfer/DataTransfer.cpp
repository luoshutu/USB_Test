// DataTransfer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "USBTransfer.h"
#include "DataTransfer.h"

CUSBTransfer usbdevice;

BOOL CloseUSBDevice()
{
	return usbdevice.CloseUSBDevice();
}

int GetUSBDeviceCount()
{
	return usbdevice.GetUSBDeviceCount();
}

//------------------------------------------------//
//----函数名：GetSerialNumber()-------------------//
//------输入：无----------------------------------//
//------输出：string allSerialNumber--------------//
//------功能：返回所有设备的serialNumber----------//
//------------------------------------------------//
string GetSerialNumber()
{
	int USBDeviceNumber = GetUSBDeviceCount();
	int serialNumberLen = 10*2;

	TCHAR *serialNumber = (TCHAR *)malloc(USBDeviceNumber*serialNumberLen*sizeof(TCHAR));
	memset(serialNumber, 0, USBDeviceNumber*serialNumberLen * sizeof(TCHAR));

	usbdevice.GetSerialNumber(serialNumber, USBDeviceNumber*serialNumberLen);

	string allSerialNumber = "";
	for (int i = 0; i < USBDeviceNumber*serialNumberLen; i++)
	{
		allSerialNumber += char(*(serialNumber + i));
	}
	//std::cout << ((USBDeviceNumber*serialNumberLen));
	return allSerialNumber;
}

 