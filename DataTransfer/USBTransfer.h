#ifndef _USBTRANSFER_H_
#define _USBTRANSFER_H_

#include "CyUsbBase.h"
#include "stdafx.h"
#pragma comment(linker,"/nodefaultlib:libcmt.lib")

class  CUSBTransfer
{
public:
	CUSBTransfer();
	~CUSBTransfer();

public:
	/*-----------����FPGA��AS����PS����----------------------*/
	BOOL	SetAPMode(BYTE nMode);
	/*------------------����FPGA-----------------------------*/
	BOOL	ConfigDevice(string lpszRbfFile);
	/*BOOL	ConfigDevice(LPCTSTR lpszRbfFile);*/
	BOOL	ConfigDevice(PBYTE pData, LONG dwLen);

	/*------------------�˿�0��ͼ�����ݻض�------------------*/
	BOOL	GetImageData(BYTE *pData,long uSize);

	/*------------------�˿�1�ļĴ�������--------------------*/
	BOOL	SetRegData(REGDATA &vData);
	BOOL	SetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag);
	BOOL	GetRegData(REGDATA &vAddr);
	BOOL	GetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag);
	
	/*------------------�˿�2�ı������-----------------------*/
	BOOL	SetDT_Data(TABLE_TYPE nType, const BYTE* pData, ULONG uSize);
	BOOL	GetDT_Data(TABLE_TYPE nType, BYTE* pData, ULONG uSize);

	/*------------------�˿�3��ͼ����������־�ض�-------------*/
	BOOL	GetImageFlag();

	/*------------------�˿�4�ı������-----------------------*/
	BOOL	GetLinkData(BYTE *pData,long uSize);

	/*------------------�˿�5�Ĳɼ���������־�ض�-------------*/
	BOOL	GetLinkFlag();

	/*------------------�˿�6����������־�ض�----------------*/
	BOOL    SetAFE5805(BYTE addr, WORD data);
public:
	BOOL	GetHWStatus();			// ��ȡӲ��״̬
	int		GetConfigProcess();		// ��ȡ���ý���
	DWORD	GetImageFrameNum();		// ��ȡ����֡����

private:
	CCyUsbBase m_usb;		// USB
	BOOL	   m_hwstatus;	// Ӳ��״̬
	int		   m_process;	// ���ý���
	IMAGEHEAD  m_imagehead; // ͼ������ͷ
	LINKHEAD   m_linkhead;  // ��·����ͷ
};


#endif

