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

BOOL OpenUSBDevice(int USBDeviceNumber)
{
	return usbdevice.OpenUSBDevice(USBDeviceNumber);
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
	int USBDeviceCount = GetUSBDeviceCount();
	// 每台设备序列号的最长长度为10个字节，TCHAR为两个字节
	int serialNumberLen = 10*2;
	// 用于存储所有设备的USB序列号
	string allSerialNumber = "";

	TCHAR *serialNumber = (TCHAR *)malloc(serialNumberLen*sizeof(TCHAR));
	memset(serialNumber, 0, serialNumberLen * sizeof(TCHAR));

	CloseUSBDevice();

	// 依次打开所有USB设备，并获取所有设备的序列号
	for (int i = 0; i < USBDeviceCount; i++)
	{
		OpenUSBDevice(i);
		usbdevice.GetSerialNumber(serialNumber, serialNumberLen);
		// 后一半为空
		for (int j = 0; j < serialNumberLen/2; j++) 
		{
			allSerialNumber += char(*(serialNumber + j));
		}
	}

	//std::cout << ((USBDeviceNumber*serialNumberLen));
	return allSerialNumber;
}

 