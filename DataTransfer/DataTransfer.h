#pragma once
#include "string"
#include "stdafx.h"

//-------�汾����Ϣ
extern "C" BOOL __declspec(dllexport) CloseUSBDevice();
extern "C" BOOL __declspec(dllexport) OpenUSBDevice(int USBDeviceNumber);
extern "C" int  __declspec(dllexport) GetUSBDeviceCount();
extern "C" string __declspec(dllexport) GetSerialNumber();
