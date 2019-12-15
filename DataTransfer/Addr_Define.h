//---------------------------------------------------------------------------
// USB�˿�
typedef enum tagPORTTYPE
{
	USBPORT_IMAGEDATA = 0,		// ͼ������				----------		0
	USBPORT_REGDATA,			// �Ĵ������Ʋ���		----------		1
	USBPORT_TABDATA,			// �������		        ----------		2
	USBPORT_IMAGEFLAG,			// ͼ����������־       ----------      3
	USBPORT_LINKDATA,			// ��·���ݵĲɼ�       ----------      4
	USBPORT_LINKFLAG,			// ��·��������־       ----------      5
	USBPORT_AFECONG,			// AFE�ļĴ�������      ----------      6
	USBPORT_7,					// 7
}PORTTYPE;



/*��������ݱ������*/
enum TABLE_TYPE {
	TT_BOOTLOADER = 0,  // FPGA bin�ļ����·�������BOOTLOADERģʽ��������ģʽ
	TT_SCANINDEX = 2,	// ɨ����ſ���
	TT_HVSWITCH  = 4,	// ��ѹ����
	TT_ATGC		 = 6,	// ģ��TGC
	TT_ENCODING	 = 8,	// �������
	TT_FILTER	 = 10,	// ��̬�˲�ϵ��
	TT_IQ		 = 12,	// IQ���Ƶ��
	TT_TRANSMIT	 = 14,	// �����ӳ�ϵ��
	TT_RECEIVE	 = 16,	// �����ӳ�ϵ��
	TT_DTGC		 = 18,	// ����TGC
	TT_BEAMWEIGHT= 20,	// ����Ȩֵ
	TT_RXWEIGHT  = 22,  // ��Ȩ����
	TT_PWDATA    = 24,
};

// ͼ��ͷ
#pragma pack(push, 1)
typedef struct tagIMAGEHEAD
{
	BYTE  flag;			//����־
	DWORD frames;		//֡������
}IMAGEHEAD, *PIMAGEHEAD;
#pragma pack(pop)

// ����ͷ����·���ݣ�
#pragma pack(push, 1)
typedef struct tagLINKHEAD
{
	BYTE  flag;			//����־
	DWORD frames;		//֡������
}LINKHEAD, *PLINKHEAD;
#pragma pack(pop)

// AFE�������ݸ�ʽ
union afeconfgdata
{
	struct 
	{
		WORD data;
		BYTE addr;
		BYTE reserve;
	};
	DWORD vData;
};

//ʹ�ýṹ�嶨��
typedef union tagREGDATA
{
	struct
	{
		BYTE nAddr;
		BYTE nValue;
	};
	WORD nData;
}REGDATA,*LPREGDATA;


enum BIT_FLAG
{
	RW_8BIT  = 1,	//8λ
	RW_16BIT = 2,	//16λ
	RW_32BIT = 4,	//32λ
};

// �������ģʽ
const BYTE TRMODE[16][4] = 
{
	{0,1,2,3},  // 0
	{0,0,0,0},  // 1
	{0,7,8,9},  // 2
	{0,0,0,0},  // 3
	{4,4,4,4},  // 4
	{5,5,5,5},  // 5
	{6,6,6,6},  // 6
	{7,7,7,7},  // 7
	{8,8,8,8},  // 8
	{9,9,9,9},  // 9
	{0,0,4,4},	// 10
	{7,7,8,8},	// 11
	{7,7,9,9},	// 12
	{8,8,9,9},	// 13
	{4,4,5,5},	// 14
	{0,4,8,9},	// 15
};

//��ַ����		

//ֻ��
#define ADDRESS_R_VERSION_YEAR			0x00		//�汾��ŵ�ַ	2013��ֻȡ13
#define ADDRESS_R_VERSION_DATE_DAY		0x01		//�汾����(��)
#define ADDRESS_R_VERSION_DATE_MONTH	0x02		//�汾����(��)
#define ADDRESS_R_VERSION_TIME_MINUTES	0x03		//�汾����(��)
#define ADDRESS_R_VERSION_TIME_HOURS	0x04		//�汾����(ʱ)	
#define ADDRESS_R_VERSION_NUMBER		0x05		//HW�汾��

#define ADDRESS_RW_TABLE_SELELCT		0x06		//�����Ƶ�ַ
#define ADDRESS_RW_FREEZE				0x07		//����

#define ADDRESS_RW_BITWITDH				0x1D		//��λ�Ĵ���

#define ADDRESS_RW_TOTALSCANLINENUM		0x0A		//��ɨ���ߣ�2��BYTE��

#define ADDRESS_RW_BEAMSTART			0x10		//��ʼɨ�裨2��BYTE��
#define ADDRESS_RW_RXSTART				0x12		//��ʼ������2��BYTE��

#define ADDRESS_RW_DMODEREPEAT			0x16		//Dģʽ�ظ�������2��BYTE��
#define ADDRESS_RW_LINKENABLE			0x18		//������·�Ĳɼ�ʹ��
#define ADDRESS_RW_CHNNUMSEL			0x19		//ͨ��ѡ��  �ɼ�ģʽΪ0-4��Ӧͨ����0-15���ɼ�ģʽΪ5ʱ����Ӧ�����ϳɺ�����
#define ADDRESS_RW_MODESEL				0x1A		//ģʽѡ�� 0-4 ͨ������   5 RF����
#define ADDRESS_RW_POSITION				0x1B		//�ߺ�λ��ѡ��

#define ADDRESS_RW_BEAMFORMMODE			0x20		//�ನ���ϳɿ��� 0-�������յ��ϳ� 1-����˫���ĺϳ�
#define ADDRESS_RW_LINEWAITTIME			0x21		//ÿ����֮��ĵȴ�ʱ�䣨2��BYTE��

#define ADDRESS_RW_RECFG_L				0xA0		//���ؾ������ʼ��ַ L-M-H
#define ADDRESS_RW_RECFG_M				0xA1
#define ADDRESS_RW_RECFG_H				0xA2

#define ADDRESS_RW_BOOTENABLE			0xA4		//��дʹ�� EPCS��������16Mbit = 2.^24
#define ADDRESS_RW_ERASE_SECTOR_START	0xA5		//����EPCS����ʼSector �ܹ�32��sector = 2.^5 ��ÿ��sector��СΪ0.5Mbit = 2.^19
#define ADDRESS_RW_ERASE_SECTOR_STOP	0xA6		//��дEPCS�Ľ���Sector

//#define ADDRESS_RW_CHANNELTXENABLE		0x34		//����ʹ�ܿ���
//#define ADDRESS_RW_CHANNELFILTERENABLE  0x1A		//�˲���ʹ�ܿ���
//#define ADDRESS_RW_CHANNELTXENABLE

#define ADDRESS_RW_ADDRESS_READ			0xFF		//����������