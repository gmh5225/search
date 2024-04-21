#include "timer.h"
#include "utils.h"

KTIMER Timer;
KDPC TimerDpc;
LONG TimerLock;

VOID TimerDpcRoutine(IN PKDPC Dpc, IN PVOID DeferredContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    InterlockedIncrement(&TimerLock);
    DbgPrint("[*] Running at %s\n", __FUNCTION__);
    StackWalk();    // �Ե�ǰ����������ִ�е��߳̽��е��ö�ջ����
    InterlockedDecrement(&TimerLock);
}

NTSTATUS StartTimerCheck()
{
    LARGE_INTEGER DueTime;
    BOOLEAN Result;

    KeInitializeDpc(&TimerDpc, TimerDpcRoutine, NULL);  // ��ʼ�� DPC ����
    KeInitializeTimer(&Timer);  // ��ʼ����ʱ��
    DueTime.QuadPart = -10000 * 1;  // ���ö�ʱ��1ms��ִ��
    Result = KeSetTimer(&Timer, DueTime, &TimerDpc);
    Sleep(10);
    return Result ? STATUS_ALREADY_COMPLETE : STATUS_SUCCESS;
}

NTSTATUS StopTimerCheck()
{
    BOOLEAN Result;
    
    Result = KeCancelTimer(&Timer);
    while (TimerLock) KeStallExecutionProcessor(10);
    return STATUS_SUCCESS;
}

NTSTATUS CheckByTimer()
{
    NTSTATUS status;

    status = StartTimerCheck();		// ������ʱ��
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status = StopTimerCheck();		// ֹͣ��ʱ��
    if (!NT_SUCCESS(status)) {
        return status;
    }
    return STATUS_SUCCESS;
}
