// USB_Test.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <setupapi.h>
#include "string"
#include "../DataTransfer/DataTransfer.h"


#pragma comment(lib, "setupapi.lib")

#ifdef  _DEBUG
#pragma comment(lib,"../x64/Debug/DataTransfer.lib")
#else
#pragma comment(lib,"../x64/Release/DataTransfer.lib")
#endif

using namespace std;


void Wchar_tToString(string& szDst, wchar_t *wchar)
{
	wchar_t * wText = wchar;
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);//WideCharToMultiByte的运用
	char *psText;  // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);//WideCharToMultiByte的再次运用
	szDst = psText;// std::string赋值
	delete[]psText;// psText的清除
}

string GetDevicePath(int deviceNumber)
{
	//Initialization

	string usbDevicePath = "";

	GUID CyDrvGuid = { 0xae18aa60, 0x7f6a, 0x11d4, 0x97, 0xdd, 0x0, 0x1, 0x2, 0x29, 0xb9, 0x59 };   // cyusb3 guid

	SP_DEVINFO_DATA devInfoData;

	SP_DEVICE_INTERFACE_DATA  devInterfaceData;

	PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData;

	ULONG requiredLength = 0;

	HDEVINFO hwDeviceInfo = SetupDiGetClassDevs((LPGUID)& CyDrvGuid,   //Returns a handle to the device information set

		NULL,

		NULL,

		DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

	if (hwDeviceInfo != INVALID_HANDLE_VALUE) { //checks if the handle is invalid

		devInterfaceData.cbSize = sizeof(devInterfaceData);         //get the size of devInterfaceData structure



		//enumerates the device interfaces that are contained in a device information set

		if (SetupDiEnumDeviceInterfaces(hwDeviceInfo, 0, (LPGUID)& CyDrvGuid,

			deviceNumber, &devInterfaceData)) {

			SetupDiGetInterfaceDeviceDetail(hwDeviceInfo, &devInterfaceData, NULL, 0,

				&requiredLength, NULL);

			ULONG predictedLength = requiredLength;

			functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(predictedLength);

			functionClassDeviceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

			devInfoData.cbSize = sizeof(devInfoData);

			//Retrieve the information from Plug and Play including the device path

			if (SetupDiGetInterfaceDeviceDetail(hwDeviceInfo,

				&devInterfaceData,

				functionClassDeviceData,

				predictedLength,

				&requiredLength,

				&devInfoData)) {

				wprintf(L"%ls\n", functionClassDeviceData->DevicePath); //Prints the device path of the required device
				
				Wchar_tToString(usbDevicePath,functionClassDeviceData->DevicePath);
				WCHAR nujn = (functionClassDeviceData->DevicePath)[39];
				wcout << nujn << endl;
			}

		}

	}

	SetupDiDestroyDeviceInfoList(hwDeviceInfo);
	return usbDevicePath;
}

int main()
{	
	int usbDeviceNumber = 0;
	usbDeviceNumber = GetUSBDeviceCount();

	string serialNumber = " ";
	serialNumber = GetSerialNumber();

	for (int i = 0; i < usbDeviceNumber; i++)
	{
		GetDevicePath(i);
	}

	cout << "USB设备数量：" << usbDeviceNumber << endl;
	cout << "USB设备的序列号：" << serialNumber << endl;

	// Exit
	cout << endl << "Finnished.";
		
	cin.get();
	//system("pause");

    return 0;
}

