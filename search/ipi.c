#include "ipi.h"
#include "utils.h"

ULONG_PTR IpiBroadcastFunction(_In_ ULONG_PTR Argument)
{
    UNREFERENCED_PARAMETER(Argument);

    DbgPrint("[*] Running at %s\n", __FUNCTION__);
    StackWalk();    // ջ����
    return 0;
}

NTSTATUS CheckByIpi()
{
    // �����������жϣ�������к�������ִ��IpiBroadcastFunction
    KeIpiGenericCall(IpiBroadcastFunction, 0);
    return STATUS_SUCCESS;
}
