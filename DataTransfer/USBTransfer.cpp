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

BOOL CUSBTransfer::OpenUSBDevice()
{
	if (m_pUsbDev == NULL)
	{
		return FALSE;
	}
	m_pUsbDev->Open(0);
	return (m_pUsbDev != NULL && m_pUsbDev->IsOpen());
}

BOOL CUSBTransfer::CloseUSBDevice()
{
	if (m_pUsbDev != NULL)
	{
		m_pUsbDev->Close();
	}
	return TRUE;
}

BOOL CUSBTransfer::USBDeviceStatus()
{
	return (m_pUsbDev != NULL && m_pUsbDev->IsOpen());
}

int CUSBTransfer::USBDeviceCount()
{
	return m_pUsbDev->DeviceCount();
}

void CUSBTransfer::GetDeviceInfo(TCHAR *lpszDst, long count)
{
	if (lpszDst == NULL || count <= 0)		return;
	if (m_pUsbDev == NULL)					return;
	if (!m_pUsbDev->IsOpen())				return;

	_tcscpy_s(lpszDst, count, m_pUsbDev->Product);
}

void CUSBTransfer::GetUSBVersion(TCHAR *lpszDst, long count)
{
	if (lpszDst == NULL || count <= 0)		return;
	if (m_pUsbDev == NULL)					return;
	if (!m_pUsbDev->IsOpen())				return;

	ULONG dwDriverVersion = m_pUsbDev->DriverVersion;

	_stprintf_s(lpszDst, count, _T("%d.%d.%d.%d"),
				(dwDriverVersion >> 24) & 0xff,
				(dwDriverVersion >> 16) & 0xff,
				(dwDriverVersion >> 8) & 0xff,
				(dwDriverVersion >> 0) & 0xff
	);

}

void CUSBTransfer::GetSerialNumber(TCHAR *lpszDst, long count)
{
	if (lpszDst == NULL || count <= 0)		return;
	if (m_pUsbDev == NULL)					return;
	if (!m_pUsbDev->IsOpen())				return;

	_tcscpy_s(lpszDst, count, m_pUsbDev->SerialNumber);
}
