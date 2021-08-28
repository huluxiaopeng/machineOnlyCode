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
* @briref ��ȡdmiͷ����Ϣ
*/
struct dmi_header
{
	BYTE type; //����
	BYTE length; //ͷ������
	WORD handle; 
	BYTE data[1]; //��������
};

/**
* @briref ��ȡdmiͷ����Ϣ
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
* @briref ����������IP����
*/
struct NET_ADAPTER_IP
{
	std::string strIP; //IP��ַ
	std::string strMask; //��������
};


/**
* @briref ��������������
*/
struct NET_ADAPTER
{
	DWORD nIndex;//ϵͳ�豸������
	std::string strName; //����
	std::string strDesc;//˵����ע
	std::string strType;//����
	std::string strMac;//mac��ַ
	std::vector<NET_ADAPTER_IP> ips;//���е�ַ

};



class MachineCode
{
public:
	MachineCode();
	~MachineCode();
	string getMachineCode();
	
protected:
private:
	//��ȡUUDI
	char *GetUUID( );
	char *hexbyte(BYTE b, char *ptr);
	bool biosuuid(unsigned char *uuid);

	//��ȡcpuID
	void GetCPUID(char* pCpuID);
	void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType);
	void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue);

	//��ȡӲ��ID
	bool GetIDEHDSerial(int driveNum, string& serialNum);// ��ȡIDEӲ�����к�(ֻ֧��Windows NT/2000/XP���ϲ���ϵͳ)
	bool GetSCSIHDSerial(int driveNum, string& serialNum);// ��ȡSCSIӲ�����к�(ֻ֧��Windows NT/2000/XP���ϲ���ϵͳ)

	//��ȡmac id

private:
	char szModelNumber[64];
	char szSerialNumber[64];

};
