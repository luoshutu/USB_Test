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
//----��������Init()------------------------------//
//------���룺��----------------------------------//
//------�������----------------------------------//
//------���ܣ���ʼ������--------------------------//
//------------------------------------------------//
void CUSBTransfer::Init()
{
	USBDeviceCount = 0;
	USBDeviceCount = m_pUsbDev->DeviceCount();	// ��ȡ��ǰ���ӵ� USB �豸����

	for (int i = 0; i < USBDeviceCount; i++)
	{
		OpenUSBDevice(i);
	}
}

//------------------------------------------------//
//----��������OpenUSBDevice()---------------------//
//------���룺int num����ǰ������USB�豸����------//
//------�����BOOL��USB�豸�Ƿ�򿪳ɹ�-----------//
//------���ܣ�������USB�豸---------------------//
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
//----��������CloseUSBDevice()--------------------//
//------���룺��----------------------------------//
//------�����BOOL��USB�豸�Ƿ�رճɹ�-----------//
//------���ܣ��ر�����USB�豸---------------------//
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
//----��������GetSerialNumber()-------------------//
//------���룺TCHAR *lpszDst, long count----------//
//------�������----------------------------------//
//------���ܣ���ȡ����USB�豸���к�---------------//
//------------------------------------------------//
void CUSBTransfer::GetSerialNumber(TCHAR *lpszDst, int count)
{
	if (lpszDst == NULL || count <= 0)		return;
	if (m_pUsbDev == NULL)					return;
	if (!m_pUsbDev->IsOpen())				return;

	memcpy(lpszDst, m_pUsbDev->SerialNumber, count);
}

//------------------------------------------------//
//----��������GetUSBDeviceCount()-----------------//
//------���룺��----------------------------------//
//------�����int��USB�豸������------------------//
//------���ܣ���ȡ����USB�豸������---------------//
//------------------------------------------------//
int CUSBTransfer::GetUSBDeviceCount()
{
	return USBDeviceCount;
}

