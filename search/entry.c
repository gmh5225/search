#include <ntifs.h>
#include "search.h"

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	StopSearch();	// ���������߳�
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	NTSTATUS status;
	
	DriverObject->DriverUnload = DriverUnload;
	status = StartSearch();		// ���������߳�
	return status;
}
