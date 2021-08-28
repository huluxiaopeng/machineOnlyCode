#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <objbase.h>
#include <intrin.h>
#include <sysinfoapi.h>
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib") 
using namespace std;

// IOCTL
#if(_WIN32_WINNT < 0x0400)
#define SMART_GET_VERSION				0x00074080
#define SMART_RCV_DRIVE_DATA			0x0007c088
#endif
#define FILE_DEVICE_SCSI				0x0000001b
#define IOCTL_SCSI_MINIPORT_IDENTIFY	((FILE_DEVICE_SCSI << 16) + 0x0501)
#define IOCTL_SCSI_MINIPORT				0x0004D008

// IDEREGS
#define IDE_ATAPI_IDENTIFY		0xA1
#define IDE_ATA_IDENTIFY		0xEC
#define IDENTIFY_BUFFER_SIZE	512
#define SENDIDLENGTH			sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE



typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;
	BYTE bRevision;
	BYTE bReserved;
	BYTE bIDEDeviceMap;
	DWORD fCapabilities;
	DWORD dwReserved[4];
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

typedef struct _IDSECTOR
{
	USHORT  wGenConfig;
	USHORT  wNumCyls;
	USHORT  wReserved;
	USHORT  wNumHeads;
	USHORT  wBytesPerTrack;
	USHORT  wBytesPerSector;
	USHORT  wSectorsPerTrack;
	USHORT  wVendorUnique[3];
	CHAR    sSerialNumber[20];
	USHORT  wBufferType;
	USHORT  wBufferSize;
	USHORT  wECCSize;
	CHAR    sFirmwareRev[8];
	CHAR    sModelNumber[40];
	USHORT  wMoreVendorUnique;
	USHORT  wDoubleWordIO;
	USHORT  wCapabilities;
	USHORT  wReserved1;
	USHORT  wPIOTiming;
	USHORT  wDMATiming;
	USHORT  wBS;
	USHORT  wNumCurrentCyls;
	USHORT  wNumCurrentHeads;
	USHORT  wNumCurrentSectorsPerTrack;
	ULONG   ulCurrentSectorCapacity;
	USHORT  wMultSectorStuff;
	ULONG   ulTotalAddressableSectors;
	USHORT  wSingleWordDMA;
	USHORT  wMultiWordDMA;
	BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;


/**
* @briref 获取dmi头部信息
*/
struct dmi_header
{
	BYTE type; //类型
	BYTE length; //头部长度
	WORD handle; 
	BYTE data[1]; //数据内容
};

/**
* @briref 获取dmi头部信息
*/
struct RawSMBIOSData {
	BYTE  Used20CallingMethod;
	BYTE  SMBIOSMajorVersion;
	BYTE  SMBIOSMinorVersion;
	BYTE  DmiRevision;
	DWORD  Length;
	BYTE  SMBIOSTableData[1];
};

/**
* @briref 网卡适配器IP属性
*/
struct NET_ADAPTER_IP
{
	std::string strIP; //IP地址
	std::string strMask; //子网掩码
};


/**
* @briref 网卡适配器属性
*/
struct NET_ADAPTER
{
	DWORD nIndex;//系统设备引索号
	std::string strName; //名称
	std::string strDesc;//说明备注
	std::string strType;//类型
	std::string strMac;//mac地址
	std::vector<NET_ADAPTER_IP> ips;//所有地址

};



class MachineCode
{
public:
	MachineCode();
	~MachineCode();
	string getMachineCode();
	
protected:
private:
	//获取UUDI
	char *GetUUID( );
	char *hexbyte(BYTE b, char *ptr);
	bool biosuuid(unsigned char *uuid);

	//获取cpuID
	void GetCPUID(char* pCpuID);
	void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType);
	void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue);

	//获取硬盘ID
	bool GetIDEHDSerial(int driveNum, string& serialNum);// 获取IDE硬盘序列号(只支持Windows NT/2000/XP以上操作系统)
	bool GetSCSIHDSerial(int driveNum, string& serialNum);// 获取SCSI硬盘序列号(只支持Windows NT/2000/XP以上操作系统)

	//获取mac id

private:
	char szModelNumber[64];
	char szSerialNumber[64];

};
