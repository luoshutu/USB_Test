#pragma once
#include "stdafx.h"

//-------∞Ê±æ∫≈–≈œ¢
extern "C" BOOL __declspec(dllexport) OpenUSBDevice();
extern "C" BOOL __declspec(dllexport) USBDeviceStatus();
extern "C" int  __declspec(dllexport) USBDeviceCount();
extern "C" TCHAR __declspec(dllexport) GetUSBVersion();
extern "C" TCHAR __declspec(dllexport) GetDeviceInfo();
