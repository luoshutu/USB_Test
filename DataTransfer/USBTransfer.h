#pragma once

#ifndef _USBTRANSFER_H_
#define _USBTRANSFER_H_

#include "./inc/CyAPI.h"
#pragma comment(linker,"/nodefaultlib:libcmt.lib")
#pragma comment(lib,"setupapi.lib")

class  CUSBTransfer
{
public:
	CUSBTransfer(void);
	virtual ~CUSBTransfer(void);
public:
	BOOL	OpenUSBDevice();		// 打开USB设备	
	BOOL	CloseUSBDevice();		// 关闭USB设备	
	BOOL	USBDeviceStatus();		// 是否已经打开USB设备
	int     USBDeviceCount();		// 检查USB设备的数量

	void GetUSBVersion(TCHAR *lpsz, long count);	// 获取USB驱动版本
	void GetDeviceInfo(TCHAR *lpsz, long count);	// 获取设备硬件版本
	//LPCTSTR GetManufacturerName(); //获取设备制造商名称
	//LPCTSTR GetProductName();     //获取产品名称
//public:
//	/*!
//	@brief 读取一个字节
//	@param  nPort: 端口号,取值[0,255]
//	@param nData: 读出的字节数据s
//	@return 成功返回ture, 否则返回false
//	*/
//	BOOL Read(BYTE nPort, BYTE & nData);
//	/*!
//	@brief 读取一串数据
//	@param  nPort: 端口号,取值[0,255]
//	@param  pBuffer: 读出的数据
//	@param  ulCount: 读取的长度
//	@return
//	*/
//	BOOL Read(BYTE nPort, PBYTE pBuffer, ULONG ulCount);
//	/*!
//	@brief 写入一个字节
//	@param nPort: 端口号,取值[0,255]
//	@param nData: 写入的数据
//	@return
//	*/
//	BOOL Write(BYTE nPort, BYTE nData);
//	/*!
//	@brief 写入一串数据
//	@param  nPort: 端口号,取值[0,255]
//	@param pBuffer: 写入的数据
//	@param ulCount: 写入的字节长度
//	@return */
//	BOOL Write(BYTE nPort, const BYTE* pBuffer, ULONG ulCount);
//
//public:
//	/*
//	@brief 从ep0端点读取一段数据
//	@param pBuffer 数据指针
//	@param ulCount 数据大小
//	@return
//	*/
//	BOOL Ep0Read(PBYTE pBuffer, ULONG ulCount);
//	///PBYTE pBuffer :数据指针	ULONG ulCount :数据大小
//	BOOL Ep0Write(const BYTE* pBuffer, ULONG ulCount);
//public:
//	BOOL Ep0WriteControl(BYTE nCommand, BYTE nPort, BYTE nIndex);
//	BOOL Ep0ReadControl(PBYTE pBuffer, BYTE nCommand, BYTE nPort, BYTE nLen);
//	BOOL SetWorkMode(BYTE nCommand);
//	BOOL SetDownloadMode(BYTE nCommand);
//	BOOL FinishDownload(BYTE nCommand);
//	BOOL EraseEPCS(BYTE nCommand);
//	BOOL ReadFpga(PBYTE pBuffer, BYTE nLen);
//	BOOL ReadEPCS(PBYTE pBuffer, BYTE nLen);
//	BOOL Ep4Write(const BYTE* pBuffer, ULONG ulCount, BYTE nEPCS_ID);
//	BOOL PowerOn();
//	BOOL ConfigDeviceAS(BYTE nEPCS_ID);
//protected:
//	BOOL ReadData(PBYTE pBuffer, ULONG uLength);
//	BOOL WriteData(const BYTE* pBuffer, ULONG uLength);
//	BOOL WriteCommand(BYTE nCommand, BYTE nPort);
//private:
//	CCyUsbBase(const CCyUsbBase&);
//	CCyUsbBase&operator=(const CCyUsbBase&);
private:
	CCyUSBDevice *m_pUsbDev;
	CRITICAL_SECTION m_cs;	//读写同步
};

#endif