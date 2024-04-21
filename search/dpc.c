#include "dpc.h"
#include "utils.h"

VOID AsynchronousDpcRoutine(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    DbgPrint("[*] Running at %s\n", __FUNCTION__);
    StackWalk();    // �Ե�ǰ����������ִ�е��߳̽��е��ö�ջ����
}

NTSTATUS CheckByAsynchronousDpc()
{
    KDPC Dpc;
    ULONG ProcessorCount;

    ProcessorCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
    for (CCHAR i = 0; i < (CCHAR)ProcessorCount; i++)
    {
        KeInitializeDpc(&Dpc, AsynchronousDpcRoutine, NULL);    // ��ʼ��KDPC�ṹ��
        KeSetTargetProcessorDpc(&Dpc, i);      // ��DPCִ�е�CPU����
        if (!KeInsertQueueDpc(&Dpc, NULL, NULL))    // ��DPC���뵽CPU���ĵ�DPC������
        {
            DbgPrint("[-] Failed to insert DPC!\n");
            return STATUS_UNSUCCESSFUL;
        }
        KeFlushQueuedDpcs();    // ʹ�����к���DPC�����ϵ�DPC��������ִ��
    }
    return STATUS_SUCCESS;
}

VOID SynchronousDpcRoutine(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    DbgPrint("[*] Running at %s\n", __FUNCTION__);
    StackWalk();    // �Ե�ǰ����������ִ�е��߳̽��е��ö�ջ����
    KeSignalCallDpcDone(SystemArgument1);   // ֪ͨDPC����ϵͳ�ú����ϵ�DPC����ִ�н���
}

NTSTATUS CheckBySynchronousDpc()
{
    // ʹ���к���ͬʱ����ִ��ָ����DPC����
    KeGenericCallDpc(SynchronousDpcRoutine, NULL);
    return STATUS_SUCCESS;
}