#include "stdafx.h"
#include "CyUsbBase.h"

#ifdef _USRDLL

//#pragma comment(lib,"./lib/x64/CyAPI.lib")
//#pragma comment(lib,"SetupApi.lib")
//#pragma message("-------使用cyapi 64位----------")


#ifdef _WIN64
#pragma comment(lib,"./lib/x64/CyAPI.lib")
#pragma comment(lib,"SetupApi.lib")
#pragma message("-------使用cyapi 64位----------")
#else
#pragma comment(lib,"./lib/x86/CyAPI.lib")
#endif
#endif

static const int SIZE_BULK_PKG			= 512;		// bulk封包长度1024
static const int SIZE_ERROR_BYTE		= 1026;		//1026	514// 每启动一次读数据命令时,返回的数据中前面的{1026}个是错误的,此常数是测试所得
static const int EE_PAGE_SIZE           = 256;

static const DWORD ADDR_EP0				= 0xE000;	//FX2 的RAM 起始地址(因为其地址空间 0xe000~0xe1ff 共512字节)
static const int SIZE_EP0_PKG			= 510;		//ep0 共512字节,首2字节作标志,故有效数据长度510
static const BYTE FPGADOWNLOAD_INIT		= 0xB5;		//fpga下载初始化
static const BYTE FPGADOWNLOAD_CONFIG	= 0xB4;		//配置fpga

static const BYTE WRITE_ENABLE_COMMAND	= 0xB2;		// 写使能
static const BYTE READ_ENABLE_COMMAND	= 0xB3;		// 读使能

static const BYTE WORKING_MODE			= 0xB8;		//工作模式 PS：0x00 AS黑白模式EPCS1：0x01 AS彩色模式EPCS2 ：0x02
static const BYTE READ_FPGA_STATE		= 0xB9;		//读取FPGA工作状态
static const BYTE VR_WRITE_SPI_BB_START = 0xC7;		//设置download
static const BYTE VR_WRITE_SPI_BB_STOP	= 0xC8;		//结束download
static const BYTE VR_ERASE_BULK			= 0xC2;		//擦除EPCS
static const BYTE VR_READ_STATUS		= 0xC3;		//读状态
static const BYTE EE_PS_ID				= 0x00;
static const BYTE EE_EPCS_ID_1			= 0x01;
static const BYTE EE_EPCS_ID_2			= 0x02;

CCyUsbBase::CCyUsbBase(void)
{
	m_pUsbDev = new CCyUSBDevice();
	::InitializeCriticalSection(&m_cs);
}

CCyUsbBase::~CCyUsbBase(void)
{
	if(m_pUsbDev!=NULL) m_pUsbDev->Close();
	delete m_pUsbDev;
	::DeleteCriticalSection(&m_cs);
}

BOOL CCyUsbBase::OpenDriver()
{
	if(m_pUsbDev == NULL) 	return FALSE;
	//m_pUsbDev->Close();
	m_pUsbDev->Open(0);
	return (m_pUsbDev != NULL && m_pUsbDev->IsOpen());
}

BOOL CCyUsbBase::CloseDriver()
{
	if(m_pUsbDev != NULL )
	{
		m_pUsbDev->Close();	
	}
	return TRUE;
}

BOOL CCyUsbBase::IsDriverOpened()
{
	return (m_pUsbDev!=NULL && m_pUsbDev->IsOpen());
}

BOOL CCyUsbBase::Ep0Read(PBYTE pBuffer, ULONG ulCount )
{
	if(m_pUsbDev == NULL|| ulCount > 512) 	return FALSE;
	//一次只能发送512字节数据,其中头2个用作标志,故数据只能一次发送510个
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept == NULL) return FALSE; 
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xA0;
	ept->Index     = 0;
	ept->Value	   = ADDR_EP0;//ep0起始地址	
	LONG len = ulCount;	
	return ept->XferData(pBuffer,len);		
}

BOOL CCyUsbBase::Ep0Write(const BYTE* pBuffer,ULONG ulCount )
{
	if (m_pUsbDev == NULL|| ulCount <= 0) 
		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept == NULL) return FALSE;

	BOOL bResult = FALSE;	
	DWORD nBytes = 0;		
	const ULONG ulFree = ulCount % SIZE_EP0_PKG; //余数部分长度512-2
	ULONG ulTimes = ulCount/SIZE_EP0_PKG;
	if(ulFree > 0) ulTimes +=1;;//计算需写入次数,如果有余数,则多写一次
	//一次只能发送512字节数据,其中头2个用作标志,故数据只能一次发送510个
	WORD sign = 0;		
	BYTE tmpBuffer[512] = { 0xAA, 0xAA };

	::EnterCriticalSection(&m_cs);
	//发送初始化命令
	this->WriteCommand(FPGADOWNLOAD_INIT, 0);
	const BYTE* pWrite = pBuffer; 
	//分成一包包写入硬件
	for(ULONG i=0; i<ulTimes; i++,pWrite += SIZE_EP0_PKG)
	{		
		//不是最后一包 或者 没有余数,则数据长度为SIZE_PACKAGE,否则为ulfree
		LONG ulDataLen = (i != ulTimes-1 || ulFree == 0) ? SIZE_EP0_PKG : ulFree;
		//复制数据到缓冲区
		::memcpy(tmpBuffer + 2, pWrite, ulDataLen);
		//写入整块数据
		LONG len = ulDataLen + 2;
		ept->Target    = TGT_DEVICE;
		ept->ReqType   = REQ_VENDOR;
		ept->Direction = DIR_TO_DEVICE;
		ept->ReqCode   = 0xA0;
		ept->Index     = 0;
		ept->Value	   = ADDR_EP0;
		if(!ept->XferData(tmpBuffer, len)) 
		{
			bResult = FALSE;
			break;
		}
		//发送开始搬运命令
		this->WriteCommand(FPGADOWNLOAD_CONFIG, 0);			
		while(TRUE)
		{	
			//读取标志位
			if(!this->Ep0Read((BYTE*)&sign, 2))	{ bResult = FALSE;goto leave;}			
			if	   (sign == 0xBBBB)				{ break;					 }//搬运完成,跳出等待继续写下一包
			else if(sign == 0xCCCC)				{ bResult = TRUE;goto leave; }	//已经写完
			else if(sign == 0xDDDD)				{ bResult = FALSE;goto leave;}	//出错了													  
			else								{ continue;					 }//继续等待
		}						
	}//end of 分成一包包循环写入
leave:
	::LeaveCriticalSection(&m_cs);
	return bResult;	
}

BOOL CCyUsbBase::Ep4Write(const BYTE* Buffer,ULONG ulCount, BYTE nEPCS_ID)
{
	if (m_pUsbDev == NULL|| ulCount <= 0) 
		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept == NULL) return FALSE;

	CCyUSBEndPoint *epBulkOut = m_pUsbDev->EndPointOf(0x04);
	if(epBulkOut == NULL) return FALSE;

	CCyUSBEndPoint *epBulkIn = m_pUsbDev->EndPointOf(0x88);
	if(epBulkIn == NULL) return FALSE;

	BOOL bResult = TRUE;
	const ULONG ulFree = ulCount % EE_PAGE_SIZE; 
	ULONG ulTimes = ulCount / EE_PAGE_SIZE;
	if(ulFree > 0) ulTimes +=1;//计算需写入次数,如果有余数,则多写一次
	const BYTE* pWrite = Buffer; 
	BYTE tmpBuffer[259] = {0};
	BYTE msb_lsb_swap[256] = {0,128,64,192,32,160,96,224,16,144,80,208,48,176,112,240,//0x00~0x0f
		8,136,72,200,40,168,104,232,24,152,88,216,56,184,120,248,//0x10~0x1f
		4,132,68,196,36,164,100, 228,20,148,84,212,52,180,116,244,//0x20~0x2f
		12,140,76,204,44,172,108,236,28,156,92,220, 60,188,124,252,//0x30~0x3f
		2,130,66,194,34,162,98,226,18,146,82,210,50,178,114,242,//0x40~0x4f
		10,138,74,202,42,170,106,234,26,154,90,218,58,186,122,250,//0x50~0x5f
		6,134,70,198,38,166,102,230,22,150,86,214,54,182,118,246,//0x60~0x6f
		14,142,78,206,46,174,110,238,30,158,94,222,62,190,126,254,//0x70~0x7f
		1,129,65,193,33,161,97,225,17,145,81,209,49,177,113,241,//0x80~0x8f
		9,137,73,201,41,169,105,233,25,153,89,217,57,185,121,249,//0x90~0x9f
		5,133,69,197,37,165,101,229,21,149,85,213,53,181,117,245,//0xa0~0xaf
		13,141,77,205,45,173,109,237,29,157,93,221,61,189,125,253,//0xb0~0xbf
		3,131,67,195,35,163,99,227,19,147,83,211,51,179,115,243,//0xc0~0xcf
		11,139,75,203,43,171,107,235,27,155,91,219,59,187,123,251,//0xd0~0xdf
		7,135,71,199,39,167,103,231,23,151,87,215,55,183,119,247,//0xe0~0xef
		15,143,79,207,47,175,111,239,31,159,95, 223,63,191,127,255};//0xf0~0xff

	BYTE pBuffer[16];
	LONG len = 16;
	//关机
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb7;
	ept->XferData(pBuffer, len);

	//开始下载
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = VR_WRITE_SPI_BB_START;
	ept->Index     = nEPCS_ID;
	ept->XferData(pBuffer, len);

	//开机
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb6;
	ept->XferData(pBuffer, len);

	//擦除
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = VR_ERASE_BULK;
	ept->XferData(pBuffer, len);

	//读状态
	BYTE nEpcs;
	do
	{
		ept->ReqCode   = VR_READ_STATUS;
		ept->XferData(pBuffer, len);
		nEpcs = pBuffer[1];
	} while ((nEpcs & 0x01) > 0);

	//循环下发数据
	for(ULONG i=0; i<ulTimes; i++)
	{
		//不是最后一包 或者 没有余数,则数据长度为SIZE_PACKAGE,否则为ulfree
		LONG ulDataLen = (i != ulTimes-1 || ulFree == 0) ? EE_PAGE_SIZE : ulFree;
		LONG xferLen = ulDataLen + 3;
		int epcs_address = i * EE_PAGE_SIZE;
		tmpBuffer[0]= (BYTE)((epcs_address & 0xFF0000)>> 16);
		tmpBuffer[1] = (BYTE)((epcs_address & 0x00FF00) >> 8);
		tmpBuffer[2] = (BYTE)(epcs_address & 0x0000FF);
		for (int j=0; j<ulDataLen; j++)
		{
			tmpBuffer[j + 3] = msb_lsb_swap[pWrite[i * EE_PAGE_SIZE + j]];
		}

		if(!epBulkOut->XferData((PUCHAR)tmpBuffer, xferLen)) 
		{
			bResult = FALSE;
			break;
		}

		do
		{
			ept->ReqCode   = VR_READ_STATUS;
			ept->XferData(pBuffer, len);
			nEpcs = pBuffer[1];
		} while ((nEpcs & 0x01) > 0);

		if(!epBulkIn->XferData((PUCHAR)tmpBuffer, xferLen)) 
		{
			bResult = FALSE;
			break;
		}

	}
	//结束下载
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = VR_WRITE_SPI_BB_STOP;
	ept->XferData(pBuffer, len);

	return bResult;
}

BOOL CCyUsbBase::ConfigDeviceAS(BYTE nEPCS_ID)
{
	if (m_pUsbDev == NULL) 
		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept == NULL) return FALSE;
	BOOL bResult = TRUE;
	BYTE pBuffer[16];
	LONG len = 16;
	//关机
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb7;
	bResult = ept->XferData(pBuffer, len);
	//设置工作模
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb8;
	ept->Value     = nEPCS_ID;
	bResult = ept->XferData(pBuffer, len);
	//开机
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb6;
	bResult = ept->XferData(pBuffer, len);

	return bResult;
}

BOOL CCyUsbBase::WriteCommand(BYTE nCommand, BYTE nPort)
{
	if (m_pUsbDev == NULL )		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept==NULL) return FALSE;
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;//DIR_TO_DEVICE;
	ept->ReqCode   = nCommand;
	ept->Value     = nPort;
	ept->Index     = 0;	

	ULONG ulBytes = 0;
	BYTE pBuffer[16];
	LONG len = 16;
	return ept->XferData(pBuffer, len);
}

BOOL CCyUsbBase::Ep0WriteControl(BYTE nCommand, BYTE nPort, BYTE nIndex)
{
	if (m_pUsbDev == NULL )		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept==NULL) return FALSE;
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_TO_DEVICE;//DIR_TO_DEVICE;
	ept->ReqCode   = nCommand;
	ept->Value     = nPort;
	ept->Index     = 0;	

	ULONG ulBytes = 0;
	BYTE pBuffer[16] = {0};
	LONG len = 1;
	pBuffer[0] = nIndex;
	return ept->XferData(pBuffer, len);
}

BOOL CCyUsbBase::Ep0ReadControl(PBYTE pBuffer, BYTE nCommand, BYTE nPort, BYTE nLen)
{
	if (m_pUsbDev == NULL )		return FALSE;
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept==NULL) return FALSE;
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = nCommand;
	ept->Value     = nPort;
	ept->Index     = 0;	

	ULONG ulBytes = 0;
	//BYTE Buffer[16];
	LONG len = nLen;
	//pBuffer[0] = 1;
	return ept->XferData(pBuffer, len);
}

BOOL CCyUsbBase::SetWorkMode(BYTE nCommand)
{
	return this->Ep0WriteControl(WORKING_MODE, 0, nCommand);
}

BOOL CCyUsbBase::SetDownloadMode(BYTE nCommand)
{
	return this->Ep0WriteControl(VR_WRITE_SPI_BB_START, 0, nCommand);
}

BOOL CCyUsbBase::FinishDownload(BYTE nCommand)
{
	return this->Ep0WriteControl(VR_WRITE_SPI_BB_STOP, 0, nCommand);
}

BOOL CCyUsbBase::EraseEPCS(BYTE nCommand)
{
	return this->Ep0WriteControl(VR_ERASE_BULK, 0, nCommand);
}

BOOL CCyUsbBase::ReadFpga(PBYTE pBuffer, BYTE nLen)
{
	return this->Ep0ReadControl(pBuffer, READ_FPGA_STATE, 0, nLen);
}

BOOL CCyUsbBase::ReadEPCS(PBYTE pBuffer, BYTE nLen)
{
	return this->Ep0ReadControl(pBuffer, VR_READ_STATUS, 0, nLen);
}

BOOL CCyUsbBase::PowerOn()
{
	return this->Ep0WriteControl(0xb6, 0, 0);
}

BOOL CCyUsbBase::ReadData(PBYTE pBuffer, ULONG uLength)
{
	if(pBuffer == NULL || uLength <= 0) return FALSE;
	if(uLength % SIZE_BULK_PKG != 0) return FALSE;
	if(uLength % SIZE_BULK_PKG != 0)//字节数必须是512的整数倍,否则会传输出错
		return FALSE;
	::ZeroMemory(pBuffer, uLength);

	CCyBulkEndPoint *pBulkIn = m_pUsbDev->BulkInEndPt;
	if(pBulkIn == NULL)		return FALSE;	

	LONG len  = uLength;		 
	return  pBulkIn->XferData(pBuffer, len);	
}

BOOL CCyUsbBase::WriteData(const BYTE* pBuffer, ULONG uLength)
{	
	if(pBuffer == NULL || uLength <= 0 || m_pUsbDev==NULL) return FALSE;
	CCyBulkEndPoint *pBulkOut = m_pUsbDev->BulkOutEndPt;
	if(pBulkOut == NULL)	return FALSE;

	LONG len  = uLength;
	return pBulkOut->XferData((PUCHAR)pBuffer, len);
}
/*
由于readdata限制读取长度必须为512字节的整数倍,而每次发出读命令后,
首1026字节数据是错误的,故先读取3*512个字节,得到并复制第一包有效数据,再读取后面的
*/
BOOL CCyUsbBase::Read(BYTE nPort, PBYTE pBuffer, ULONG ulCount)
{
	if(pBuffer == NULL|| ulCount <= 0) return FALSE;	
	::EnterCriticalSection(&m_cs);
	BOOL bRes = FALSE;
	do 
	{	
		if (!this->WriteCommand(READ_ENABLE_COMMAND, nPort))		break;
		//首次读取比错误字节多点的基数整数倍长度的数据512*3
		const ULONG LEN_FST_PKG = (SIZE_ERROR_BYTE % SIZE_BULK_PKG > 0) ? ((SIZE_ERROR_BYTE / SIZE_BULK_PKG + 1) * SIZE_BULK_PKG ) : SIZE_ERROR_BYTE;
		BYTE btFst3pkg[LEN_FST_PKG];//先读取512*3字节
		if(!this->ReadData(btFst3pkg, LEN_FST_PKG))					break;
		//复制前3包中的有效数据,长度为512*3-1026 或者 ulcount
		const LONG LEN_3 = LEN_FST_PKG - SIZE_ERROR_BYTE;
		//如果长度比len_3还小,则复制并返回
		if(ulCount <= LEN_3) 
		{
			::memcpy(pBuffer,btFst3pkg + SIZE_ERROR_BYTE, ulCount);
		}
		else
		{
			::memcpy(pBuffer, btFst3pkg + SIZE_ERROR_BYTE, LEN_3);
			//剩余需读取数据长度
			ULONG lenLeft = ulCount-LEN_3;		
			ULONG lenLast = lenLeft % SIZE_BULK_PKG;//余数
			ULONG lenMiddle = lenLeft - lenLast;//减于余数后的长度
			//读取中间包
			if(lenMiddle > 0 )
			{
				//读取数据
				if(!this->ReadData(pBuffer + LEN_3, lenMiddle))			break;
			}
			//最后一包
			if(lenLast > 0)
			{ 
				//读一个包大小的数据
				if(!this->ReadData(btFst3pkg, SIZE_BULK_PKG))			break;
				::memcpy(pBuffer + LEN_3 + lenMiddle, btFst3pkg, lenLast);
			}
		}
		bRes = TRUE;
	} while (FALSE);
	::LeaveCriticalSection(&m_cs);
	return bRes;		
}

BOOL CCyUsbBase::Write(BYTE nPort, const BYTE* pBuffer, ULONG ulCount)
{
	if(pBuffer == NULL || ulCount <= 0 || m_pUsbDev==NULL) return FALSE;
	BOOL bRes = FALSE;
	::EnterCriticalSection(&m_cs);	
	if(this->WriteCommand(WRITE_ENABLE_COMMAND, nPort))
	{
		bRes = this->WriteData(pBuffer, ulCount);
	}	
	::LeaveCriticalSection(&m_cs);
	return bRes;
}

BOOL CCyUsbBase::Read(BYTE nPort, BYTE & nData)
{
	return this->Read(nPort, &nData, 1);
}
BOOL CCyUsbBase::Write(BYTE nPort, BYTE nData)
{
	return this->Write( nPort, &nData, 1);
}

static bool AnsiToUnicode(const char *strA, WCHAR* strW,long count) 
{
	//计算从Ansi转换到Unicode后需要的字节数//要转换的Ansi字符串//自动计算长度
	int nMinLen = MultiByteToWideChar( CP_ACP,MB_COMPOSITE,(LPCSTR)strA, -1, NULL,	0);
	if(count <= nMinLen) return false;
	//为转换后保存Unicode字符串分配内存//从Ansi转换到Unicode字符 //转换到tmpstr//最多转换widecharlen个Unicode字符
	MultiByteToWideChar( CP_ACP,	MB_COMPOSITE, (LPCSTR)strA,	-1,	strW, nMinLen );	
	return true;
}

void CCyUsbBase::GetDeviceInfo(TCHAR *lpszDst, long count)
{
	if(lpszDst == NULL || count <= 0) return;
	if(m_pUsbDev == NULL)    return ;
	if(!m_pUsbDev->IsOpen()) return ;
	_tcscpy_s(lpszDst, count, m_pUsbDev->Product);
}

void CCyUsbBase::GetUSBVersion(TCHAR *lpszDst, long count)
{
	if(lpszDst == NULL || count <= 0) return;
	if(m_pUsbDev == NULL)    return ;
	if(!m_pUsbDev->IsOpen()) return ;

	ULONG dwDriverVersion = m_pUsbDev->DriverVersion; 
	_stprintf_s(lpszDst, count, _T("%d.%d.%d.%d"), 
		(dwDriverVersion>>24)&0xff,
		(dwDriverVersion>>16)&0xff,
		(dwDriverVersion>>8)&0xff,
		(dwDriverVersion>>0)&0xff
		);

}

