// machineOnlyCode.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "machineOnlyCode.h"
using namespace std;
MachineCode::MachineCode()
{

}

MachineCode::~MachineCode()
{

}

string MachineCode::getMachineCode()
{
	//UUID
	char uuid[40] = {0};
	char *getUuid;
	getUuid = GetUUID();
	strcpy(uuid, getUuid);
	cout << "BIOS UUID:" << uuid << endl;

	//CPUID
	char getCPUID[32] = "";
	GetCPUID(getCPUID);
	cout << "CPUID:" << getCPUID << endl;

	//DISKID
	string getDISKID;
	for (int driveNum = 0; driveNum < 5; driveNum++) 
	{
		if(!GetIDEHDSerial(driveNum, getDISKID))
		{
			GetSCSIHDSerial(driveNum, getDISKID);
		}
		if (!getDISKID.empty())
			break;
	}
	cout << "HardDisk serialNum:" << getDISKID << endl;

	return string();
}

char * MachineCode::hexbyte(BYTE b, char * ptr)
{
	static const char *digits{ "0123456789ABCDEF" };
	*ptr++ = digits[b >> 4];
	*ptr++ = digits[b & 0x0f];
	return ptr;
}

bool MachineCode::biosuuid(unsigned char * uuid)
{
	bool result = false;
	RawSMBIOSData *smb = nullptr;
	BYTE *data;

	DWORD size = 0;

	// Get size of BIOS table
	size = GetSystemFirmwareTable('RSMB', 0, smb, size);
	smb = (RawSMBIOSData*)malloc(size);

	// Get BIOS table
	GetSystemFirmwareTable('RSMB', 0, smb, size);

	//Go through BIOS structures
	data = smb->SMBIOSTableData;
	while (data < smb->SMBIOSTableData + smb->Length)
	{
		BYTE *next;
		dmi_header *h = (dmi_header*)data;

		if (h->length < 4)
			break;

		//Search for System Information structure with type 0x01 (see para 7.2)
		if (h->type == 0x01 && h->length >= 0x19)
		{
			data += 0x08; //UUID is at offset 0x08

						  // check if there is a valid UUID (not all 0x00 or all 0xff)
			bool all_zero = true, all_one = true;
			for (int i = 0; i < 16 && (all_zero || all_one); i++)
			{
				if (data[i] != 0x00) all_zero = false;
				if (data[i] != 0xFF) all_one = false;
			}
			if (!all_zero && !all_one)
			{
				/* As off version 2.6 of the SMBIOS specification, the first 3 fields
				of the UUID are supposed to be encoded on little-endian. (para 7.2.1) */
				*uuid++ = data[3];
				*uuid++ = data[2];
				*uuid++ = data[1];
				*uuid++ = data[0];
				*uuid++ = data[5];
				*uuid++ = data[4];
				*uuid++ = data[7];
				*uuid++ = data[6];
				for (int i = 8; i < 16; i++)
					*uuid++ = data[i];

				result = true;
			}
			break;
		}

		//skip over formatted area
		next = data + h->length;

		//skip over unformatted area of the structure (marker is 0000h)
		while (next < smb->SMBIOSTableData + smb->Length && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;
		data = next;
	}
	free(smb);
	return result;
}

char * MachineCode::GetUUID()
{
	BYTE uuid[16];
	char getUuid[40];
	char *ptr = getUuid;
	if (biosuuid(uuid))
	{
		int i;
		for (i = 0; i < 4; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 6; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 8; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 10; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = '-';
		for (; i < 16; i++)
			ptr = hexbyte(uuid[i], ptr);
		*ptr++ = 0;
		return getUuid;
	}
	return nullptr;
}


void MachineCode::GetCPUID(char* pCpuID)
{
	int dwBuf[4];
	getcpuid((unsigned int *)dwBuf,1);
	sprintf(pCpuID, "%08X", dwBuf[3]);
	sprintf(pCpuID + 8, "%08X", dwBuf[0]);
	return ;
}

void MachineCode::getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
	__cpuid((int*)(void*)CPUInfo, (int)(InfoType));
	//getcpuidex(CPUInfo, InfoType, 0);
}


void MachineCode::getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER) // MSVC  
#if defined(_WIN64) // 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.  
	__cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else  
	if (NULL == CPUInfo)
		return;
	_asm {
		// load. 读取参数到寄存器.  
		mov edi, CPUInfo;
		mov eax, InfoType;
		mov ecx, ECXValue;
		// CPUID  
		cpuid;
		// save. 将寄存器保存到CPUInfo  
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
#endif  
#endif  
}

bool MachineCode::GetIDEHDSerial(int driveNum, string & serialNum)
{
	BYTE IdOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	bool bFlag = false;
	char driveName[32];
	HANDLE hDevice = 0;

	sprintf_s(driveName, 32, "\\\\.\\PhysicalDrive%d", driveNum);
	// 创建文件需要管理员权限
	hDevice = CreateFileA(driveName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice != INVALID_HANDLE_VALUE) {
		GETVERSIONOUTPARAMS versionParams;
		DWORD bytesReturned = 0;
		// 得到驱动器的IO控制器版本
		memset((void*)&versionParams, 0, sizeof(versionParams));
		if (DeviceIoControl(hDevice, SMART_GET_VERSION, NULL, 0,
			&versionParams, sizeof(versionParams), &bytesReturned, NULL))
		{
			if (versionParams.bIDEDeviceMap > 0) {
				BYTE bIDCmd = 0;   // IDE或者ATAPI识别命令
				SENDCMDINPARAMS scip;

				// 如果驱动器是光驱，采用命令IDE_ATAPI_IDENTIFY
				// 否则采用命令IDE_ATA_IDENTIFY读取驱动器信息
				bIDCmd = (versionParams.bIDEDeviceMap >> driveNum & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
				memset(&scip, 0, sizeof(scip));
				memset(IdOutCmd, 0, sizeof(IdOutCmd));
				// 为读取设备信息准备参数
				scip.cBufferSize = IDENTIFY_BUFFER_SIZE;
				scip.irDriveRegs.bFeaturesReg = 0;
				scip.irDriveRegs.bSectorCountReg = 1;
				scip.irDriveRegs.bSectorNumberReg = 1;
				scip.irDriveRegs.bCylLowReg = 0;
				scip.irDriveRegs.bCylHighReg = 0;
				// 计算驱动器位置
				scip.irDriveRegs.bDriveHeadReg = 0xA0 | (((BYTE)driveNum & 1) << 4);
				// 设置读取命令
				scip.irDriveRegs.bCommandReg = bIDCmd;
				scip.bDriveNumber = (BYTE)driveNum;
				scip.cBufferSize = IDENTIFY_BUFFER_SIZE;

				// 读取驱动器信息
				if (DeviceIoControl(hDevice, SMART_RCV_DRIVE_DATA,
					(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1, (LPVOID)&IdOutCmd,
					sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
					&bytesReturned, NULL))
				{
					USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

					int nIndex = 0, nPosition = 0;
					char szSeq[32] = { 0 };
					for (nIndex = 10; nIndex < 20; nIndex++) {
						szSeq[nPosition] = (unsigned char)(pIdSector[nIndex] / 256);
						nPosition++;
						szSeq[nPosition] = (unsigned char)(pIdSector[nIndex] % 256);
						nPosition++;
					}
					serialNum = szSeq;
					serialNum.erase(0, serialNum.find_first_not_of(" "));
					bFlag = true;  // 读取硬盘信息成功
				}
				else
					cout << "DeviceIoControl error:" << GetLastError() << endl;
			}
			else
				cout << "bIDEDeviceMap <= 0" << endl;
		}
		else
			cout << "DeviceIoControl VERSION error:" << GetLastError() << endl;
		CloseHandle(hDevice);  // 关闭句柄
	}
	else
		cout << "CreateFileA error:" << GetLastError() << endl;
	return bFlag;
}

bool MachineCode::GetSCSIHDSerial(int driveNum, string & serialNum)
{
	bool bFlag = false;
	int controller = driveNum;
	HANDLE hDevice = 0;
	char driveName[32];
	sprintf_s(driveName, 32, "\\\\.\\Scsi%d:", controller);
	hDevice = CreateFileA(driveName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice != INVALID_HANDLE_VALUE) {
		DWORD dummy;
		for (int drive = 0; drive < 2; drive++) {
			char buffer[sizeof(SRB_IO_CONTROL) + SENDIDLENGTH];
			SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
			SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
			// 准备参数
			memset(buffer, 0, sizeof(buffer));
			p->HeaderLength = sizeof(SRB_IO_CONTROL);
			p->Timeout = 10000;
			p->Length = SENDIDLENGTH;
			p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
			strncpy_s((char *)p->Signature, 9, "SCSIDISK", 9);
			pin->irDriveRegs.bCommandReg = IDE_ATA_IDENTIFY;
			pin->bDriveNumber = drive;
			// 得到SCSI硬盘信息
			if (DeviceIoControl(hDevice, IOCTL_SCSI_MINIPORT, buffer,
				sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
				buffer, sizeof(SRB_IO_CONTROL) + SENDIDLENGTH, &dummy, NULL))
			{
				SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
				IDSECTOR *pId = (IDSECTOR *)(pOut->bBuffer);
				if (pId->sModelNumber[0]) {
					USHORT *pIdSector = (USHORT *)pId;
					int nIndex = 0, nPosition = 0;
					char szSeq[32] = { 0 };
					for (nIndex = 10; nIndex < 20; nIndex++) {
						szSeq[nPosition] = (unsigned char)(pIdSector[nIndex] / 256);
						nPosition++;
						szSeq[nPosition] = (unsigned char)(pIdSector[nIndex] % 256);
						nPosition++;
					}
					serialNum = szSeq;
					serialNum.erase(0, serialNum.find_first_not_of(" "));
					bFlag = true;  // 读取硬盘信息成功
					break;
				}
			}
		}
		CloseHandle(hDevice);  // 关闭句柄
	}
	return bFlag;
}


int main()
{
	MachineCode onlyCode;
	onlyCode.getMachineCode();

	system("pause");
	printf("hello world");
	return 1;
}

