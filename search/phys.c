#include "phys.h"
#include "utils.h"

#pragma warning(disable:6385)

extern PCHAR PageSignature;

NTSTATUS ScanPhysicalMemory()
{
	NTSTATUS status;
	UNICODE_STRING usPhysicalMemory;
	OBJECT_ATTRIBUTES ObjectAttributes;
	HANDLE PhysicalMemoryHandle;
	PPHYSICAL_MEMORY_RANGE PhysicalMemoryRanges;
	PVOID BaseAddress;
	SIZE_T PhysicalMemorySize;
	LARGE_INTEGER SectionOffset;
	ULONG_PTR MappedAddress;
	ULONG_PTR CurrentAddress;
	ULONG_PTR PhysicalAddress;

	RtlInitUnicodeString(&usPhysicalMemory, L"\\Device\\PhysicalMemory");
	InitializeObjectAttributes(&ObjectAttributes, &usPhysicalMemory, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	status = ZwOpenSection(&PhysicalMemoryHandle, SECTION_ALL_ACCESS, &ObjectAttributes);	// �������ڴ��������
	if (!NT_SUCCESS(status)) 
	{
		return status;
	}
	// ��ȡϵͳ�����ڴ��������ݲ�����ÿ������
	PhysicalMemoryRanges = MmGetPhysicalMemoryRanges();
	while (PhysicalMemoryRanges->NumberOfBytes.QuadPart)
	{
		BaseAddress = NULL;
		SectionOffset = PhysicalMemoryRanges->BaseAddress;
		PhysicalMemorySize = PhysicalMemoryRanges->NumberOfBytes.QuadPart;
		// ӳ�������ڴ浽�����ַ
		status = ZwMapViewOfSection(
			PhysicalMemoryHandle, 
			ZwCurrentProcess(), 
			&BaseAddress, 
			0, 
			0,
			&SectionOffset,
			&PhysicalMemorySize, 
			ViewShare,
			0, 
			PAGE_READWRITE
		);
		if (!NT_SUCCESS(status)) {
			ZwClose(PhysicalMemoryHandle);
			return status;
		}
		MappedAddress = (ULONG_PTR)BaseAddress;		// ӳ��������ڴ����ʼ�����ַ
		// ��4K����ķ�ʽ����ӳ������������ڴ�
		for (LONG_PTR i = 0; i < PhysicalMemoryRanges->NumberOfBytes.QuadPart; i+= PAGE_SIZE)
		{
			CurrentAddress = MappedAddress + i;		// �ж������ַ���Ƿ����shellcode����
			if (!memcmp((PVOID)CurrentAddress, PageSignature, 19))
			{
				PhysicalAddress = SectionOffset.QuadPart + i;
				DbgPrint("[+] Shellcode mapped at PhysicalAddress:%llX -> VA:%p    <------ Shellcode!\n", 
					PhysicalAddress, GetVirtualForPhysical(PhysicalAddress));
			}
		}
		status = ZwUnmapViewOfSection(ZwCurrentProcess(), BaseAddress);		// ��������ڴ�ӳ��
		if (!NT_SUCCESS(status)) {
			ZwClose(PhysicalMemoryHandle);
			return status;
		}
		PhysicalMemoryRanges++;
	}
	return ZwClose(PhysicalMemoryHandle);
}
