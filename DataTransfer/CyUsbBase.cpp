#include "stdafx.h"
#include "CyUsbBase.h"

#ifdef _USRDLL

//#pragma comment(lib,"./lib/x64/CyAPI.lib")
//#pragma comment(lib,"SetupApi.lib")
//#pragma message("-------ʹ��cyapi 64λ----------")


#ifdef _WIN64
#pragma comment(lib,"./lib/x64/CyAPI.lib")
#pragma comment(lib,"SetupApi.lib")
#pragma message("-------ʹ��cyapi 64λ----------")
#else
#pragma comment(lib,"./lib/x86/CyAPI.lib")
#endif
#endif

static const int SIZE_BULK_PKG			= 512;		// bulk�������1024
static const int SIZE_ERROR_BYTE		= 1026;		//1026	514// ÿ����һ�ζ���������ʱ,���ص�������ǰ���{1026}���Ǵ����,�˳����ǲ�������
static const int EE_PAGE_SIZE           = 256;

static const DWORD ADDR_EP0				= 0xE000;	//FX2 ��RAM ��ʼ��ַ(��Ϊ���ַ�ռ� 0xe000~0xe1ff ��512�ֽ�)
static const int SIZE_EP0_PKG			= 510;		//ep0 ��512�ֽ�,��2�ֽ�����־,����Ч���ݳ���510
static const BYTE FPGADOWNLOAD_INIT		= 0xB5;		//fpga���س�ʼ��
static const BYTE FPGADOWNLOAD_CONFIG	= 0xB4;		//����fpga

static const BYTE WRITE_ENABLE_COMMAND	= 0xB2;		// дʹ��
static const BYTE READ_ENABLE_COMMAND	= 0xB3;		// ��ʹ��

static const BYTE WORKING_MODE			= 0xB8;		//����ģʽ PS��0x00 AS�ڰ�ģʽEPCS1��0x01 AS��ɫģʽEPCS2 ��0x02
static const BYTE READ_FPGA_STATE		= 0xB9;		//��ȡFPGA����״̬
static const BYTE VR_WRITE_SPI_BB_START = 0xC7;		//����download
static const BYTE VR_WRITE_SPI_BB_STOP	= 0xC8;		//����download
static const BYTE VR_ERASE_BULK			= 0xC2;		//����EPCS
static const BYTE VR_READ_STATUS		= 0xC3;		//��״̬
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
	//һ��ֻ�ܷ���512�ֽ�����,����ͷ2��������־,������ֻ��һ�η���510��
	CCyControlEndPoint *ept = m_pUsbDev->ControlEndPt;
	if(ept == NULL) return FALSE; 
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xA0;
	ept->Index     = 0;
	ept->Value	   = ADDR_EP0;//ep0��ʼ��ַ	
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
	const ULONG ulFree = ulCount % SIZE_EP0_PKG; //�������ֳ���512-2
	ULONG ulTimes = ulCount/SIZE_EP0_PKG;
	if(ulFree > 0) ulTimes +=1;;//������д�����,���������,���дһ��
	//һ��ֻ�ܷ���512�ֽ�����,����ͷ2��������־,������ֻ��һ�η���510��
	WORD sign = 0;		
	BYTE tmpBuffer[512] = { 0xAA, 0xAA };

	::EnterCriticalSection(&m_cs);
	//���ͳ�ʼ������
	this->WriteCommand(FPGADOWNLOAD_INIT, 0);
	const BYTE* pWrite = pBuffer; 
	//�ֳ�һ����д��Ӳ��
	for(ULONG i=0; i<ulTimes; i++,pWrite += SIZE_EP0_PKG)
	{		
		//�������һ�� ���� û������,�����ݳ���ΪSIZE_PACKAGE,����Ϊulfree
		LONG ulDataLen = (i != ulTimes-1 || ulFree == 0) ? SIZE_EP0_PKG : ulFree;
		//�������ݵ�������
		::memcpy(tmpBuffer + 2, pWrite, ulDataLen);
		//д����������
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
		//���Ϳ�ʼ��������
		this->WriteCommand(FPGADOWNLOAD_CONFIG, 0);			
		while(TRUE)
		{	
			//��ȡ��־λ
			if(!this->Ep0Read((BYTE*)&sign, 2))	{ bResult = FALSE;goto leave;}			
			if	   (sign == 0xBBBB)				{ break;					 }//�������,�����ȴ�����д��һ��
			else if(sign == 0xCCCC)				{ bResult = TRUE;goto leave; }	//�Ѿ�д��
			else if(sign == 0xDDDD)				{ bResult = FALSE;goto leave;}	//������													  
			else								{ continue;					 }//�����ȴ�
		}						
	}//end of �ֳ�һ����ѭ��д��
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
	if(ulFree > 0) ulTimes +=1;//������д�����,���������,���дһ��
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
	//�ػ�
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb7;
	ept->XferData(pBuffer, len);

	//��ʼ����
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = VR_WRITE_SPI_BB_START;
	ept->Index     = nEPCS_ID;
	ept->XferData(pBuffer, len);

	//����
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb6;
	ept->XferData(pBuffer, len);

	//����
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = VR_ERASE_BULK;
	ept->XferData(pBuffer, len);

	//��״̬
	BYTE nEpcs;
	do
	{
		ept->ReqCode   = VR_READ_STATUS;
		ept->XferData(pBuffer, len);
		nEpcs = pBuffer[1];
	} while ((nEpcs & 0x01) > 0);

	//ѭ���·�����
	for(ULONG i=0; i<ulTimes; i++)
	{
		//�������һ�� ���� û������,�����ݳ���ΪSIZE_PACKAGE,����Ϊulfree
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
	//��������
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
	//�ػ�
	ept->Target    = TGT_DEVICE;
	ept->ReqType   = REQ_VENDOR;
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb7;
	bResult = ept->XferData(pBuffer, len);
	//���ù���ģ
	ept->Direction = DIR_FROM_DEVICE;
	ept->ReqCode   = 0xb8;
	ept->Value     = nEPCS_ID;
	bResult = ept->XferData(pBuffer, len);
	//����
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
	if(uLength % SIZE_BULK_PKG != 0)//�ֽ���������512��������,����ᴫ�����
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
����readdata���ƶ�ȡ���ȱ���Ϊ512�ֽڵ�������,��ÿ�η����������,
��1026�ֽ������Ǵ����,���ȶ�ȡ3*512���ֽ�,�õ������Ƶ�һ����Ч����,�ٶ�ȡ�����
*/
BOOL CCyUsbBase::Read(BYTE nPort, PBYTE pBuffer, ULONG ulCount)
{
	if(pBuffer == NULL|| ulCount <= 0) return FALSE;	
	::EnterCriticalSection(&m_cs);
	BOOL bRes = FALSE;
	do 
	{	
		if (!this->WriteCommand(READ_ENABLE_COMMAND, nPort))		break;
		//�״ζ�ȡ�ȴ����ֽڶ��Ļ������������ȵ�����512*3
		const ULONG LEN_FST_PKG = (SIZE_ERROR_BYTE % SIZE_BULK_PKG > 0) ? ((SIZE_ERROR_BYTE / SIZE_BULK_PKG + 1) * SIZE_BULK_PKG ) : SIZE_ERROR_BYTE;
		BYTE btFst3pkg[LEN_FST_PKG];//�ȶ�ȡ512*3�ֽ�
		if(!this->ReadData(btFst3pkg, LEN_FST_PKG))					break;
		//����ǰ3���е���Ч����,����Ϊ512*3-1026 ���� ulcount
		const LONG LEN_3 = LEN_FST_PKG - SIZE_ERROR_BYTE;
		//������ȱ�len_3��С,���Ʋ�����
		if(ulCount <= LEN_3) 
		{
			::memcpy(pBuffer,btFst3pkg + SIZE_ERROR_BYTE, ulCount);
		}
		else
		{
			::memcpy(pBuffer, btFst3pkg + SIZE_ERROR_BYTE, LEN_3);
			//ʣ�����ȡ���ݳ���
			ULONG lenLeft = ulCount-LEN_3;		
			ULONG lenLast = lenLeft % SIZE_BULK_PKG;//����
			ULONG lenMiddle = lenLeft - lenLast;//����������ĳ���
			//��ȡ�м��
			if(lenMiddle > 0 )
			{
				//��ȡ����
				if(!this->ReadData(pBuffer + LEN_3, lenMiddle))			break;
			}
			//���һ��
			if(lenLast > 0)
			{ 
				//��һ������С������
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
	//�����Ansiת����Unicode����Ҫ���ֽ���//Ҫת����Ansi�ַ���//�Զ����㳤��
	int nMinLen = MultiByteToWideChar( CP_ACP,MB_COMPOSITE,(LPCSTR)strA, -1, NULL,	0);
	if(count <= nMinLen) return false;
	//Ϊת���󱣴�Unicode�ַ��������ڴ�//��Ansiת����Unicode�ַ� //ת����tmpstr//���ת��widecharlen��Unicode�ַ�
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

