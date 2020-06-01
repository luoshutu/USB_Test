#include "stdafx.h"
#include "USBTransfer.h"

#ifdef _WIN64
	#pragma comment(lib,"./lib/x64/CyAPI.lib")
#else
	#pragma comment(lib,"./lib/x86/CyAPI.lib")
#endif

CUSBTransfer::CUSBTransfer()
{
	m_pUsbDev = new CCyUSBDevice();
	Init();
	::InitializeCriticalSection(&m_cs);
}

CUSBTransfer::~CUSBTransfer()
{
	if (m_pUsbDev != NULL)
	{
		m_pUsbDev->Close();
	}
	delete m_pUsbDev;
	::DeleteCriticalSection(&m_cs);
}

//------------------------------------------------//
//----函数名：Init()------------------------------//
//------输入：无----------------------------------//
//------输出：无----------------------------------//
//------功能：初始化变量--------------------------//
//------------------------------------------------//
void CUSBTransfer::Init()
{
	USBDeviceCount = 0;
	USBDeviceCount = m_pUsbDev->DeviceCount();	// 获取当前连接的 USB 设备数量

	for (int i = 0; i < USBDeviceCount; i++)
	{
		OpenUSBDevice(i);
	}
}

//------------------------------------------------//
//----函数名：OpenUSBDevice()---------------------//
//------输入：int num，当前所连接USB设备数量------//
//------输出：BOOL，USB设备是否打开成功-----------//
//------功能：打开所有USB设备---------------------//
//------------------------------------------------//
BOOL CUSBTransfer::OpenUSBDevice(int USBNumber)
{
	if (m_pUsbDev == NULL)
	{
		return FALSE;
	}

	m_pUsbDev->Open(USBNumber);

	return (m_pUsbDev != NULL && m_pUsbDev->IsOpen());
}

//------------------------------------------------//
//----函数名：CloseUSBDevice()--------------------//
//------输入：无----------------------------------//
//------输出：BOOL，USB设备是否关闭成功-----------//
//------功能：关闭所有USB设备---------------------//
//------------------------------------------------//
BOOL CUSBTransfer::CloseUSBDevice()
{
	if (m_pUsbDev != NULL)
	{
		m_pUsbDev->Close();
	}
	return TRUE;
}

//------------------------------------------------//
//----函数名：GetSerialNumber()-------------------//
//------输入：TCHAR *lpszDst, long count----------//
//------输出：无----------------------------------//
//------功能：获取所有USB设备序列号---------------//
//------------------------------------------------//
void CUSBTransfer::GetSerialNumber(TCHAR *lpszDst, int count)
{
	if (lpszDst == NULL || count <= 0)		return;
	if (m_pUsbDev == NULL)					return;
	if (!m_pUsbDev->IsOpen())				return;

	memcpy(lpszDst, m_pUsbDev->SerialNumber, count);
}

//------------------------------------------------//
//----函数名：GetUSBDeviceCount()-----------------//
//------输入：无----------------------------------//
//------输出：int，USB设备的数量------------------//
//------功能：获取所有USB设备的数量---------------//
//------------------------------------------------//
int CUSBTransfer::GetUSBDeviceCount()
{
	return USBDeviceCount;
}

