#include <ntifs.h>
#ifdef __cplusplus
extern "C"{
#endif


#include <ntddk.h>
#include "ShadowSSDT.h"
#include "GetActiveWindow.h"


	VOID DriverUnload(PDRIVER_OBJECT)
	{
		KdPrint(("驱动卸载\n"));
	}


	typedef ULONG_PTR(__fastcall *pfNtUserGetThreadState)(
		IN ULONG ThreadState);


	ULONG_PTR NtUserGetThreadState(ULONG ThreadState)
	{
		PVOID ThreadStateProcAddr = NULL;

		ThreadStateProcAddr = GetShadowSSDTProcAddr(0x3);
		KdPrint(("NtUserGetThreadState的地址为%p\n", ThreadStateProcAddr));


		NTSTATUS status;
		
		PRKAPC_STATE apcState;
		
		PEPROCESS eprocess;
		
		status = PsLookupProcessByProcessId((HANDLE)GetPidByProcName((PWCHAR)L"csrss.exe"), &eprocess);
		
		if (!NT_SUCCESS(status))
		{
			return NULL;
		}
		
		apcState = (PRKAPC_STATE)ExAllocatePool(NonPagedPool, sizeof(KAPC_STATE));
		
		KeStackAttachProcess((PRKPROCESS)eprocess, apcState);



		PETHREAD ethread = PsGetCurrentThread();
		
		// ULONGLONG win32Thread = *(PULONGLONG)((UCHAR)ethread + 0x1c8);
		//
		ULONG_PTR hActiveWindow = 0;
		//
		// if (win32Thread)
		// {
		// 	// 这里蓝屏，在获取当前线程信息时，rax返回为0，所以在返回句柄时出现内存访问错误
		//
		hActiveWindow = ((pfNtUserGetThreadState)ThreadStateProcAddr)(ThreadState);
		// }
		
		
		
		
		KeUnstackDetachProcess(apcState);
		ExFreePool(apcState);
		
		ObDereferenceObject(eprocess);
		
		return hActiveWindow;
		return 0;
	}


	BOOLEAN Sleep(IN int milliSeconds)
	{
		LARGE_INTEGER Interval;
		Interval.QuadPart = -10000 * (LONGLONG)milliSeconds;

		if (KeGetCurrentIrql() == PASSIVE_LEVEL)
		{
			KeDelayExecutionThread(KernelMode, FALSE, &Interval);
		}
		else
		{
			KEVENT waitEvent;
			KeInitializeEvent(&waitEvent, NotificationEvent, FALSE);
			KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, &Interval);
		}
		return TRUE;

	}


	VOID
		StartRoutine(
		_In_ PVOID StartContext
		)
	{
		while (TRUE)
		{
			HANDLE hActiveWindow = (HANDLE)NtUserGetThreadState(1);

			Sleep(1000);
		}

	}


	NTSTATUS DispatchCommon(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{
		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = STATUS_SUCCESS;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}


	NTSTATUS DispatchDeviceIoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{
		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = STATUS_SUCCESS;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
	{

		HANDLE hActiveWindow = (HANDLE)NtUserGetThreadState(1);

		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = STATUS_SUCCESS;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}


	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;


		DriverObject->DriverUnload = DriverUnload;


		status = STATUS_SUCCESS;
		//
		// HANDLE threadHandle;
		//
		// PsCreateSystemThread(&threadHandle,
		// 	0,
		// 	NULL,
		// 	NULL,
		// 	NULL,
		// 	StartRoutine,
		// 	NULL);

		// HANDLE hActiveWindow = (HANDLE)NtUserGetThreadState(0);


		UNICODE_STRING DeviceName;
		RtlInitUnicodeString(&DeviceName, L"\\Device\\ActiveWindow");

		PDEVICE_OBJECT DeviceObject = NULL;

		IoCreateDevice(DriverObject,
			0,
			&DeviceName,
			FILE_DEVICE_UNKNOWN,
			0,
			FALSE,
			&DeviceObject);

		UNICODE_STRING SymbolicLinkName;
		RtlInitUnicodeString(&SymbolicLinkName, L"\\??\\ActiveWindow");

		IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);


		for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		{
			DriverObject->MajorFunction[i] = DispatchCommon;
		}

		DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
		

		KdPrint(("驱动加载\n"));

		return status;
	}


#ifdef __cplusplus
}
#endif

