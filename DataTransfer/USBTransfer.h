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
	/*-----------设置FPGA的AS或者PS配置----------------------*/
	BOOL	SetAPMode(BYTE nMode);
	/*------------------配置FPGA-----------------------------*/
	BOOL	ConfigDevice(string lpszRbfFile);
	/*BOOL	ConfigDevice(LPCTSTR lpszRbfFile);*/
	BOOL	ConfigDevice(PBYTE pData, LONG dwLen);

	/*------------------端口0的图像数据回读------------------*/
	BOOL	GetImageData(BYTE *pData,long uSize);

	/*------------------端口1的寄存器数据--------------------*/
	BOOL	SetRegData(REGDATA &vData);
	BOOL	SetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag);
	BOOL	GetRegData(REGDATA &vAddr);
	BOOL	GetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag);
	
	/*------------------端口2的表格数据-----------------------*/
	BOOL	SetDT_Data(TABLE_TYPE nType, const BYTE* pData, ULONG uSize);
	BOOL	GetDT_Data(TABLE_TYPE nType, BYTE* pData, ULONG uSize);

	/*------------------端口3的图像数据满标志回读-------------*/
	BOOL	GetImageFlag();

	/*------------------端口4的表格数据-----------------------*/
	BOOL	GetLinkData(BYTE *pData,long uSize);

	/*------------------端口5的采集数据满标志回读-------------*/
	BOOL	GetLinkFlag();

	/*------------------端口6的数据满标志回读----------------*/
	BOOL    SetAFE5805(BYTE addr, WORD data);
public:
	BOOL	GetHWStatus();			// 获取硬件状态
	int		GetConfigProcess();		// 获取配置进度
	DWORD	GetImageFrameNum();		// 获取数据帧计数

private:
	CCyUsbBase m_usb;		// USB
	BOOL	   m_hwstatus;	// 硬件状态
	int		   m_process;	// 配置进度
	IMAGEHEAD  m_imagehead; // 图像数据头
	LINKHEAD   m_linkhead;  // 链路数据头
};


#endif

