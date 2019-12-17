// DataTransfer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "USBTransfer.h"
#include "DataTransfer.h"

CUSBTransfer usbdevice;

BOOL OpenUSBDevice()
{
	BOOL status = usbdevice.OpenUSBDevice();

	return status;
}

BOOL USBDeviceStatus()
{
	BOOL status = usbdevice.USBDeviceStatus();

	return status;
}

int USBDeviceCount()
{
	return usbdevice.USBDeviceCount();
}
/*
void GetUSBVersion(TCHAR *lpsz, long count)
{

}*/

TCHAR GetDeviceInfo()
{
	long count = 100;
	TCHAR usbDeviceInfo = 0;
	TCHAR *deviceInfo = (TCHAR *)malloc(count * sizeof(TCHAR));
	memset(deviceInfo, 0, count * sizeof(TCHAR));

	usbdevice.GetDeviceInfo(deviceInfo, count);

	usbDeviceInfo = *deviceInfo;

	return usbDeviceInfo;
}

TCHAR GetUSBVersion()
{
	long count = 100;
	TCHAR *USBVersion = (TCHAR *)malloc(count * sizeof(TCHAR));
	memset(USBVersion, 0, count * sizeof(TCHAR));

	usbdevice.GetDeviceInfo(USBVersion, count);

	return *USBVersion;
}
