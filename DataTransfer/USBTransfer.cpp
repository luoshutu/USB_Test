// USBTransfer.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "USBTransfer.h"

CUSBTransfer::CUSBTransfer()
{

}

CUSBTransfer::~CUSBTransfer()
{

}
BOOL CUSBTransfer::SetAPMode(BYTE nMode)
{
	return m_usb.ConfigDeviceAS(nMode);
}

BOOL CUSBTransfer::ConfigDevice(string lpszRbfFile)
{

	if(lpszRbfFile.empty() ||lpszRbfFile.length() < 2) return FALSE; 

	// 读取bin文件	
	vector<BYTE>	dataSend;
	long			dwReadBytes = 0;
	
	FILE *fp = fopen(lpszRbfFile.c_str(),"rb");
	//FILE *fp = fopen(lpszRbfFile.c_str(),"r");

	if (fp != NULL)
	{
		fseek(fp,0,SEEK_END);			//定位到文件末
		dwReadBytes = ftell(fp);        //文件长度

		dataSend.resize(dwReadBytes, 0);

		fseek(fp,0,SEEK_SET);			//定位到文件前
		fread(&dataSend[0],dwReadBytes,1,fp);

		BOOL bOK = this->ConfigDevice(&dataSend[0], dataSend.size());

		fclose(fp);
		return bOK;
	}
	else
	{
		fclose(fp);
		return FALSE;
	}

}

//BOOL CUSBTransfer::ConfigDevice(LPCTSTR lpszRbfFile)
//{
//	if(lpszRbfFile==NULL || _tcslen(lpszRbfFile) < 2) return FALSE; 
//	//if (m_bExistHard) return TRUE;
//	// 读取rbf文件	
//	vector<BYTE> dataSend;
//	DWORD dwReadBytes = 0;
//	HANDLE handle = ::CreateFile(lpszRbfFile, GENERIC_READ, FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
//	if (handle == INVALID_HANDLE_VALUE) return FALSE;
//	bool bRead = false;
//	do{				
//		DWORD filesizehigh = 0;
//		DWORD filesize = ::GetFileSize(handle,&filesizehigh);
//		if(filesize <= 0 || filesizehigh > 0)						break;
//		dataSend.resize(filesize, 0);
//		if(!::ReadFile(handle, &dataSend[0],filesize, &dwReadBytes,NULL))  break;
//		if(dwReadBytes != filesize)									break;
//		bRead = true;
//	}while(FALSE);
//	::CloseHandle(handle);	
//	if(!bRead) return FALSE;
//	BOOL bOK = this->ConfigDevice(&dataSend[0], dataSend.size());
//	return bOK;
//}


BOOL CUSBTransfer::ConfigDevice(PBYTE pData, LONG dwLen)
{
	// 硬件工作状态
	m_hwstatus = FALSE;

	// USB工作状态
	do
	{
		m_process = 0;
		// 检测USB
		// 尝试5次来打开USB,防止开机启动时打开usb失败
		for(int i=0;i <10; i++)
		{
			if (this->m_usb.IsDriverOpened()) this->m_usb.CloseDriver();
			if (this->m_usb.IsDriverOpened()) break;
			if(this->m_usb.OpenDriver()) break;
			Sleep(300);
		}

		m_process = 20;
		//正在配置设备.下载FPGA程序
		BOOL bConfig = FALSE;
		if ((pData != NULL) && (dwLen > 0))
		{
			for (int nTimes=0; nTimes<5; nTimes++)	// 最多连续配置5次
			{				
				if( this->m_usb.Ep0Write(pData, dwLen) )//下载成功
				{
					bConfig = TRUE;	
					break;
				}
			}
		}
		if (!bConfig) break;	
		m_process = 50;
		Sleep(200);
		//检测设备
		BOOL bFpga = FALSE;	
		const TCHAR write[] = _T("ultimedical"); 
		TCHAR read[20]={0};

		//检查控制参数端口的读写是否有错
		if (this->m_usb.Write(USBPORT_REGDATA, (PBYTE)write,sizeof(write)))
		{
			if (this->m_usb.Read(USBPORT_REGDATA, (PBYTE)read, sizeof(write)))
			{
				if (!memcmp(write, read, _tcslen(write)))
				{
					bFpga = TRUE;
				}
			}
		}
		bFpga = TRUE;

		m_process = 70; 
		Sleep(200);		
		// 正在检测运行环境
		BOOL bCheck = FALSE;
		m_process = 80;
		bCheck = TRUE;
		m_process = 90;	
		Sleep(200);
		m_process = 100;	
		Sleep(200);
		m_hwstatus = TRUE;
	}while(FALSE);	
	return m_hwstatus;
}

BOOL CUSBTransfer::GetImageData(BYTE *pData,long uSize)
{
	BOOL bOK = FALSE;

	//while(TRUE)
	//{
		if (this->GetImageFlag())
		{
			bOK = m_usb.Read(USBPORT_IMAGEDATA, (PBYTE)pData, uSize);
		}
		else
		{
			bOK = FALSE;
		}

		return bOK;
	//}
}





BOOL CUSBTransfer::SetRegData(REGDATA &vData)
{

	BOOL bOK = m_usb.Write(USBPORT_REGDATA,(BYTE*)&vData,sizeof(REGDATA));

	return bOK;
}

BOOL CUSBTransfer::GetRegData(REGDATA &vData)
{
	return m_usb.Read(USBPORT_REGDATA, (BYTE*)&vData, sizeof(REGDATA));
}


BOOL CUSBTransfer::SetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag)
{
	if (uSize != bitFlag) return FALSE;		//数据长度标识

	REGDATA data;
	memset(&data, 0, sizeof(REGDATA));
	BOOL bOk = TRUE;
	int iRepeat = 0;

	//判断读取的是多少位数据	以8位进行分割	若要写入的数据为32位，则分4次读取
	switch(bitFlag)
	{
	case RW_8BIT:		iRepeat = 1;		break;
	case RW_16BIT:		iRepeat	= 2;		break;	
	case RW_32BIT:		iRepeat = 4;		break;		
	default:			return FALSE;
	}

	for (int index=0; index<iRepeat; index++)
	{
		//向硬件写入一个读取指令
		data.nAddr = vAddr+index;
		data.nValue = pData[index];
		bOk = bOk & this->SetRegData(data);
	}
	return bOk;
}

BOOL CUSBTransfer::GetRegData(BYTE vAddr, BYTE *pData, long uSize, BIT_FLAG bitFlag)
{
	if (uSize != bitFlag) return FALSE;		//数据长度标识

	REGDATA data;
	memset(&data, 0, sizeof(REGDATA));
	BOOL bOk = TRUE;
	int iRepeat = 0;

	//判断读取的是多少位数据	以8位进行分割	若要写入的数据为32位，则分4次读取
	switch(bitFlag)
	{
		case RW_8BIT:		iRepeat = 1;		break;
		case RW_16BIT:		iRepeat	= 2;		break;	
		case RW_32BIT:		iRepeat = 4;		break;		
		default:			return FALSE;
	}

	for (int index=0; index<iRepeat; index++)
	{
		//向硬件写入一个读取指令
		data.nAddr = ADDRESS_RW_ADDRESS_READ;
		data.nValue = vAddr+index;
		bOk = bOk & this->SetRegData(data);

		//读取数据
		//data.nValue = pData[index];
		bOk = bOk & this->GetRegData(data);
		pData[index] = data.nValue;
	}
	return bOk;
}



BOOL CUSBTransfer::SetDT_Data(TABLE_TYPE nType, const BYTE* pData, ULONG uSize)
{
	REGDATA data;
	data.nAddr  = ADDRESS_RW_TABLE_SELELCT;
	data.nValue = nType;
	if (!this->SetRegData(data)) return FALSE;

	if (!m_usb.Write(USBPORT_TABDATA, pData, uSize)) return FALSE;

	return TRUE;
}


BOOL CUSBTransfer::GetDT_Data(TABLE_TYPE nType, BYTE* pData, ULONG uSize)
{
	REGDATA data;
	data.nAddr  = ADDRESS_RW_TABLE_SELELCT;
	data.nValue = nType;

	if (!this->SetRegData(data)) return FALSE;

	if (!m_usb.Read(USBPORT_TABDATA, (PBYTE)pData, uSize)) return FALSE;

	return TRUE;
}

BOOL CUSBTransfer::GetImageFlag()
{
	BOOL bOK;
	bOK = this->m_usb.Read(USBPORT_IMAGEFLAG, (PBYTE)&m_imagehead, sizeof(IMAGEHEAD));

	bOK = bOK && (m_imagehead.flag == 1);

	return bOK;
}

BOOL CUSBTransfer::GetLinkData(BYTE *pData,long uSize)
{
	BOOL bOK = FALSE;


	bOK = m_usb.Read(USBPORT_LINKDATA, (PBYTE)pData, uSize);


	return bOK;

}

BOOL CUSBTransfer::GetLinkFlag()
{
	BOOL bOK;
	bOK = this->m_usb.Read(USBPORT_LINKFLAG, (PBYTE)&m_linkhead, sizeof(IMAGEHEAD));

	bOK = bOK && (m_linkhead.flag == 1);

	return bOK;
}

// 整体配置初始化AFE5816
BOOL CUSBTransfer::SetAFE5805(BYTE addr, WORD data)
{
	afeconfgdata AFEData;

	AFEData.addr = addr;
	AFEData.data = data;

	return m_usb.Write(USBPORT_AFECONG,(BYTE*)&AFEData,sizeof(AFEData));
}


BOOL CUSBTransfer::GetHWStatus()
{
	return m_hwstatus;
}

int CUSBTransfer::GetConfigProcess()
{
	return m_process;
}

DWORD CUSBTransfer::GetImageFrameNum()
{
	return m_imagehead.frames;
}