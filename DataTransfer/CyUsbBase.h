
#ifndef _CYUSBBASE_H_
#define  _CYUSBBASE_H_

#include "./inc/CyAPI.h"


class  CCyUsbBase
{
public:
	CCyUsbBase(void);
	virtual ~CCyUsbBase(void);
public:
	BOOL	OpenDriver();		// ��USB�豸	
	BOOL	CloseDriver();		// �ر�USB�豸	
	BOOL	IsDriverOpened();	// �Ƿ��Ѿ���USB�豸

	virtual void GetUSBVersion(TCHAR *lpsz, long count);	// ��ȡUSB�����汾
	virtual void GetDeviceInfo(TCHAR *lpsz, long count);	//��ȡ�豸Ӳ���汾
	//LPCTSTR GetManufacturerName(); //��ȡ�豸����������
	//LPCTSTR GetProductName();     //��ȡ��Ʒ����
public:
	/*!
	@brief ��ȡһ���ֽ�
	@param  nPort: �˿ں�,ȡֵ[0,255]
	@param nData: �������ֽ�����s
	@return �ɹ�����ture, ���򷵻�false
	*/
	BOOL Read(BYTE nPort, BYTE & nData);
	/*!
	@brief ��ȡһ������
	@param  nPort: �˿ں�,ȡֵ[0,255]
	@param  pBuffer: ����������
	@param  ulCount: ��ȡ�ĳ���
	@return 
	*/
	BOOL Read(BYTE nPort, PBYTE pBuffer, ULONG ulCount);
	/*!
	@brief д��һ���ֽ�
	@param nPort: �˿ں�,ȡֵ[0,255]
	@param nData: д�������
	@return 
	*/
	BOOL Write(BYTE nPort, BYTE nData);
	/*!
	@brief д��һ������
	@param  nPort: �˿ں�,ȡֵ[0,255]
	@param pBuffer: д�������
	@param ulCount: д����ֽڳ���
	@return */
	BOOL Write(BYTE nPort, const BYTE* pBuffer, ULONG ulCount);	
	
public:
	/*
	@brief ��ep0�˵��ȡһ������
	@param pBuffer ����ָ��
	@param ulCount ���ݴ�С
	@return 
	*/
	BOOL Ep0Read(PBYTE pBuffer,	ULONG ulCount	);
	///PBYTE pBuffer :����ָ��	ULONG ulCount :���ݴ�С
	BOOL Ep0Write(const BYTE* pBuffer,ULONG ulCount	);
public:
	BOOL Ep0WriteControl(BYTE nCommand, BYTE nPort, BYTE nIndex);
	BOOL Ep0ReadControl(PBYTE pBuffer, BYTE nCommand, BYTE nPort, BYTE nLen);
	BOOL SetWorkMode(BYTE nCommand);
	BOOL SetDownloadMode(BYTE nCommand);
	BOOL FinishDownload(BYTE nCommand);
	BOOL EraseEPCS(BYTE nCommand);
	BOOL ReadFpga(PBYTE pBuffer, BYTE nLen);
	BOOL ReadEPCS(PBYTE pBuffer, BYTE nLen);
	BOOL Ep4Write(const BYTE* pBuffer,ULONG ulCount,BYTE nEPCS_ID);
	BOOL PowerOn();
	BOOL ConfigDeviceAS(BYTE nEPCS_ID);
protected:
	BOOL ReadData(PBYTE pBuffer, ULONG uLength);
	BOOL WriteData(const BYTE* pBuffer, ULONG uLength);
	BOOL WriteCommand(BYTE nCommand, BYTE nPort);
private:
	CCyUsbBase(const CCyUsbBase&);
	CCyUsbBase&operator=(const CCyUsbBase&);
private:
	CCyUSBDevice *m_pUsbDev;
	CRITICAL_SECTION m_cs;	//��дͬ��
};

#endif