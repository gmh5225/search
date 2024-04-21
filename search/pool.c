#include "pool.h"
#include "import.h"
#include "utils.h"

#pragma warning(disable: 6011)
#pragma warning(disable: 6387)

// ���Pool�ڴ��Tag��������
VOID CheckPoolMemory(PVOID VirtualAddress, SIZE_T RegionSize, ULONG Tag)
{
	NTSTATUS status;
	PVOID Buffer;
	ULONG_PTR Result;
	ULONG_PTR BaseAddress;
	SIZE_T SizeCopied;
	MM_COPY_ADDRESS MmCopyAddress;

	if (Tag == 'ace0')
	{
		BaseAddress = (ULONG_PTR)VirtualAddress & ~1ull;		// ���ڴ�ʵ�ʵ���ʼ��ַ
		Buffer = ExAllocatePool(NonPagedPoolNx, RegionSize);	// �����ڴ滺����
		if (!Buffer)
			return;
		// ����Ŀ����ڴ棬���ں�����ȡ
		MmCopyAddress.VirtualAddress = (PVOID)BaseAddress;
		status = MmCopyMemory(Buffer, MmCopyAddress, RegionSize, MM_COPY_MEMORY_VIRTUAL, &SizeCopied);
		if (NT_SUCCESS(status))
		{
			// �������ڴ棬Ѱ��Shellcode����
			Result = FindPattern((PVOID)Buffer, RegionSize, "41 B8 CE 0A 00 00");	// mov  r8d, 0ACEh
			if (Result)
			{
				DbgPrint("[+] Shellcode mapped at:%llX  Size:%llX    <------ Shellcode!\n", BaseAddress, RegionSize);
			}
			ExFreePool(Buffer);
		}
	}
}

// ����ϵͳ������Pool�ڴ�
NTSTATUS ScanBigPool()
{
	NTSTATUS status;
	PSYSTEM_BIGPOOL_INFORMATION SystemBigPoolInfo;
	PSYSTEM_BIGPOOL_ENTRY Allocation;
	ULONG Length;

	// ��ȡ�������������
	status = ZwQuerySystemInformation(SystemBigPoolInformation, &Length, 0, &Length);
	if (!NT_SUCCESS(status) && status != STATUS_INFO_LENGTH_MISMATCH)
	{
		return status;
	}
	SystemBigPoolInfo = NULL;
	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		if (SystemBigPoolInfo)
		{
			ExFreePool(SystemBigPoolInfo);
		}
		SystemBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)ExAllocatePool(NonPagedPoolNx, Length);
		if (!SystemBigPoolInfo)
		{
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		// ��ȡPool�ڴ���Ϣ
		status = ZwQuerySystemInformation(SystemBigPoolInformation, SystemBigPoolInfo, Length, &Length);
	}
	if (NT_SUCCESS(status))
	{
		for (ULONG i = 0; i < SystemBigPoolInfo->Count; i++)
		{
			Allocation = &SystemBigPoolInfo->AllocatedInfo[i];
			// ����ڴ������Ƿ�ΪĿ��
			CheckPoolMemory(Allocation->VirtualAddress, Allocation->SizeInBytes, Allocation->TagUlong);
		}
	}
	ExFreePool(SystemBigPoolInfo);
	return status;
}
