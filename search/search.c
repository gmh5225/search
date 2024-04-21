#include "search.h"
#include "utils.h"
#include "thread.h"
#include "dpc.h"
#include "nmi.h"
#include "ipi.h"
#include "pool.h"
#include "page.h"
#include "phys.h"
#include "timer.h"

KEVENT SearchSyncEvent;
PETHREAD SearchThread = NULL;

VOID SearchThreadProc(_In_ PVOID StartContext)
{
	UNREFERENCED_PARAMETER(StartContext);
	NTSTATUS status;
	LARGE_INTEGER Interval;

	while (TRUE)
	{
		// ͨ����ȫ�����Ĳ����첽ִ�е�DPC���̴��ִ���еĴ��벢�����ö�ջ
		CheckByAsynchronousDpc();
		
		// ��ȫ������ͬ��ִ��DPC���̴��ִ���еĴ��벢�����ö�ջ
		CheckBySynchronousDpc();
		
		// ͨ��ע��NMI�ص���������ȫ�����ķ���NMI���ִ���еĴ��벢�����ö�ջ
		CheckByNmi();		// Sometime BSOD	// ò����NMI�ص���û��DbgPrint
		
		// ͨ����ȫ�����ķ���IPI�����������жϣ��ķ�ʽ���ִ���еĴ��벢�����ö�ջ
		CheckByIpi();
		
		// �����ں�Pool�ڴ�Ѱ��shellcode�����Զ�λshellcode
		ScanBigPool();
		
		// ����ϵͳ����ҳ���Ա��ڴ涨λshellcode
		ScanPageTable();
		
		// ����ϵͳ���������ڴ棬�Ա��ڴ涨λshellcode
		ScanPhysicalMemory();

		// ͨ���ڶ�ʱ���ص�ջ���ݲ���shellcode
		CheckByTimer();

		Interval.QuadPart = -10000LL * 100;		// ���õȴ�ͬ���¼��ĳ�ʱʱ��Ϊ100ms
		status = KeWaitForSingleObject(&SearchSyncEvent, Executive, KernelMode, FALSE, &Interval);
		if (status == STATUS_SUCCESS) 
		{
			PsTerminateSystemThread(STATUS_SUCCESS);
		}
	}
}

NTSTATUS StartSearch()
{
	NTSTATUS status;
	HANDLE Handle;

	if (SearchThread) {
		return STATUS_ALREADY_COMPLETE;
	}
	KeInitializeEvent(&SearchSyncEvent, NotificationEvent, FALSE);
	status = PsCreateSystemThread(&Handle, THREAD_ALL_ACCESS, NULL, NtCurrentProcess(), NULL, SearchThreadProc, NULL);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	DbgPrint("[Search] Search started!\n");
	status = ObReferenceObjectByHandle(Handle, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&SearchThread, NULL);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	status = ZwClose(Handle);
	if (!NT_SUCCESS(status)) {
		return status;
	}
	return status;
}

NTSTATUS StopSearch()
{
	NTSTATUS status = STATUS_SUCCESS;
	if (SearchThread)
	{
		KeSetEvent(&SearchSyncEvent, 0, FALSE);		// ����ͬ���¼���״̬
		status = KeWaitForSingleObject(SearchThread, Executive, KernelMode, FALSE, NULL);	// �ȴ��߳̽���
		ObDereferenceObject(SearchThread);
		SearchThread = NULL;
		DbgPrint("[Search] Search stoped!\n");
		return status;
	}
	return status;
}
