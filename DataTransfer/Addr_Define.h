//---------------------------------------------------------------------------
// USB端口
typedef enum tagPORTTYPE
{
	USBPORT_IMAGEDATA = 0,		// 图像数据				----------		0
	USBPORT_REGDATA,			// 寄存器控制参数		----------		1
	USBPORT_TABDATA,			// 表格数据		        ----------		2
	USBPORT_IMAGEFLAG,			// 图像数据满标志       ----------      3
	USBPORT_LINKDATA,			// 链路数据的采集       ----------      4
	USBPORT_LINKFLAG,			// 链路数据满标志       ----------      5
	USBPORT_AFECONG,			// AFE的寄存器配置      ----------      6
	USBPORT_7,					// 7
}PORTTYPE;



/*发射的数据表格类型*/
enum TABLE_TYPE {
	TT_BOOTLOADER = 0,  // FPGA bin文件的下发，采用BOOTLOADER模式，即工厂模式
	TT_SCANINDEX = 2,	// 扫描序号控制
	TT_HVSWITCH  = 4,	// 高压开关
	TT_ATGC		 = 6,	// 模拟TGC
	TT_ENCODING	 = 8,	// 脉冲编码
	TT_FILTER	 = 10,	// 动态滤波系数
	TT_IQ		 = 12,	// IQ解调频率
	TT_TRANSMIT	 = 14,	// 发射延迟系数
	TT_RECEIVE	 = 16,	// 接收延迟系数
	TT_DTGC		 = 18,	// 数字TGC
	TT_BEAMWEIGHT= 20,	// 波束权值
	TT_RXWEIGHT  = 22,  // 加权数据
	TT_PWDATA    = 24,
};

// 图像头
#pragma pack(push, 1)
typedef struct tagIMAGEHEAD
{
	BYTE  flag;			//满标志
	DWORD frames;		//帧计数器
}IMAGEHEAD, *PIMAGEHEAD;
#pragma pack(pop)

// 数据头（链路数据）
#pragma pack(push, 1)
typedef struct tagLINKHEAD
{
	BYTE  flag;			//满标志
	DWORD frames;		//帧计数器
}LINKHEAD, *PLINKHEAD;
#pragma pack(pop)

// AFE配置数据格式
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

//使用结构体定义
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
	RW_8BIT  = 1,	//8位
	RW_16BIT = 2,	//16位
	RW_32BIT = 4,	//32位
};

// 发射接收模式
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

//地址定义		

//只读
#define ADDRESS_R_VERSION_YEAR			0x00		//版本年号地址	2013，只取13
#define ADDRESS_R_VERSION_DATE_DAY		0x01		//版本日期(天)
#define ADDRESS_R_VERSION_DATE_MONTH	0x02		//版本日期(月)
#define ADDRESS_R_VERSION_TIME_MINUTES	0x03		//版本日期(分)
#define ADDRESS_R_VERSION_TIME_HOURS	0x04		//版本日期(时)	
#define ADDRESS_R_VERSION_NUMBER		0x05		//HW版本号

#define ADDRESS_RW_TABLE_SELELCT		0x06		//表格控制地址
#define ADDRESS_RW_FREEZE				0x07		//冻结

#define ADDRESS_RW_BITWITDH				0x1D		//截位寄存器

#define ADDRESS_RW_TOTALSCANLINENUM		0x0A		//总扫描线（2个BYTE）

#define ADDRESS_RW_BEAMSTART			0x10		//起始扫描（2个BYTE）
#define ADDRESS_RW_RXSTART				0x12		//起始波束（2个BYTE）

#define ADDRESS_RW_DMODEREPEAT			0x16		//D模式重复次数（2个BYTE）
#define ADDRESS_RW_LINKENABLE			0x18		//数据链路的采集使能
#define ADDRESS_RW_CHNNUMSEL			0x19		//通道选择  采集模式为0-4对应通道号0-15，采集模式为5时，对应波束合成后数据
#define ADDRESS_RW_MODESEL				0x1A		//模式选择 0-4 通道数据   5 RF数据
#define ADDRESS_RW_POSITION				0x1B		//线号位置选择

#define ADDRESS_RW_BEAMFORMMODE			0x20		//多波束合成开关 0-单发单收单合成 1-单发双收四合成
#define ADDRESS_RW_LINEWAITTIME			0x21		//每条线之后的等待时间（2个BYTE）

#define ADDRESS_RW_RECFG_L				0xA0		//重载镜像的起始地址 L-M-H
#define ADDRESS_RW_RECFG_M				0xA1
#define ADDRESS_RW_RECFG_H				0xA2

#define ADDRESS_RW_BOOTENABLE			0xA4		//擦写使能 EPCS的容量：16Mbit = 2.^24
#define ADDRESS_RW_ERASE_SECTOR_START	0xA5		//擦除EPCS的起始Sector 总共32个sector = 2.^5 ，每个sector大小为0.5Mbit = 2.^19
#define ADDRESS_RW_ERASE_SECTOR_STOP	0xA6		//擦写EPCS的结束Sector

//#define ADDRESS_RW_CHANNELTXENABLE		0x34		//发射使能控制
//#define ADDRESS_RW_CHANNELFILTERENABLE  0x1A		//滤波器使能控制
//#define ADDRESS_RW_CHANNELTXENABLE

#define ADDRESS_RW_ADDRESS_READ			0xFF		//读数据命令