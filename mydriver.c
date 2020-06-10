#include "mydriver.h"

#define NDEBUG
#include <debug.h>

// I/O Manager uses three different methods to arrange
// the buffer that needs to be given to WriteFile and ReadFile.
// "Direct I/O", "Buffered I/O" and "Neither"

NTSTATUS
Example_ReadDirectIO(PDEVICE_OBJECT DeviceObject,
					 PIRP Irp)
{
	NTSTATUS NtStatus;
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	PCHAR pWriteDataBuffer;

	DbgPrint("Example_ReadDirectIO Called\r\n");

	/*
	 * Each time the IRP is passed down
	 * the driver stack a new stack location is added
	 * specifying certain parameters for the IRP
	 * to the driver.
	 */

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	if(pIoStackIrp)
	{
		
	}

	return NtStatus;
}

NTSTATUS
Example_WriteNeither(PDEVICE_OBJECT DeviceObject,
					 PIRP Irp)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	PCHAR pWriteDataBuffer;

	DbgPrint("Example_WriteNeither Called \r\n");

	/*
	 * Each time the IRP is passed down
	 * the driver stack a new stack location is added
	 * specifying certain parameters for the IRP to 
	 * the driver.
	 */

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	if(pIoStackIrp)
	{
		/*
		 * We need this in an exception handler or else 
		 * could trap
		 */

		__try {
			ProbeForRead(Irp->UserBuffer,
						 pIoStackIrp->Parameters.Write.Length,
						 TYPE_ALIGNMENT(char)
						 );
			// Checks that a user-mode buffer actually resides in the
			// user portion of the address space, and is correctly aligned.

			pWriteDataBuffer = Irp->UserBuffer;

			if(pWriteDataBuffer)
			{
				/*
				 * We need to verify that the string
				 * is NULL terminated. Bad things can happen
				 * if we access memory not vaild while in the Kernel.
				 */

				if(Example_IsStringTerminated(pWriteDataBuffer,
											  pIoStackIrp->Parameters.Write.Length))
				{
					DbgPrint(pWriteDataBuffer);
				}
			}
		} __except( EXCEPTION_EXECUTE_HANDLER ) {
			NtStatus = GetExceptionCode();
		}
	}

	return NtStatus;
}

NTSTATUS
Example_WriteDirectIO(PDEVICE_OBJECT DeviceObject,
					  PIRP Irp)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	PCHAR pWriteDataBuffer;

	DbgPrint("Example_WriteDirectIO Called \r\n");

	/*
	 * Each time the IRP is passed down
	 * the driver stack a new stack location
	 * is added specifying certain parameters
	 * for the IRP to the driver.
	 */

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	if(pIoStackIrp)
	{
		// MDL = Memory Descriptor List
		// Description of the user mode addresses and how
		// they map to physical addresses.

		pWriteDataBuffer = MmGetSystemAddressForMdlSafe(
							Irp->MdlAddress,
							NormalPagePriority);
		// Gives us a system virtual address which we can use
		// to read the mmemory. Simply maps the physical pages
		// used by the user mode process into system memory.

		// Some drivers do not always process a user mode request
		// in the context of the thread or even the process in
		// which it was issued.


		if(pWriteDataBuffer)
		{
			/*
			 * We need to verify that the string
			 * is NULL terminated. Bad things can happen
			 * if we access memory not valid while in the Kernel
			 */

			if(Example_IsStringTerminated(pWriteDataBuffer,
										  pIoStackIrp->Parameters.Write.Length))
			{
				DbgPrint(pWriteDataBuffer);
			}
		}
	}

	return NtStatus;
}

NTSTATUS
Example_WriteBufferedIO(PDEVICE_OBJECT DeviceObject,
								 PIRP Irp)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	PCHAR pWriteDataBuffer;

	DbgPrint("Example_WriteBufferedIO Called \r\n");

	/*
	 * Each time the IRP is passed down
	 * the driver stack a new stack location is added
	 * specifying certain parameters for the IRP to the
	 * driver.
	 */
	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	if(pIoStackIrp)
	{
		pWriteDataBuffer = (PCHAR) Irp->AssociatedIrp.SystemBuffer;

		if(pWriteDataBuffer)
		{
			/*
			 * We need to verify that the string
			 * is NULL terminated. Bad things can happen
			 * if we access memory not valid while in the Kernel.
			 */
			if(Example_IsStringTerminated(pWriteDataBuffer,
										  pIoStackIrp->Parameters.Write.Length))
			{
				DbgPrint(pWriteDataBuffer);
			}
		}
	}

	return NtStatus;
}

VOID Example_Unload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING usDosDeviceName;

	DbgPrint("Example_Unload Called \r\n");

	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\Example");
	IoDeleteSymbolicLink(&usDosDeviceName);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS
NTAPI
DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	UINT uiIndex = 0;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING usDriverName, usDosDeviceName;

	DbgPrint("DriverEntry Called \r\n");

	// initialises a UNICODE_STRING data structure
	RtlInitUnicodeString(&usDriverName, L"\\Device\\Example");
	RtlInitUnicodeString(&usDosDeviceName, L"\\DosDevices\\Example");

	NtStatus = IoCreateDevice(pDriverObject, 0, // 0 is the number of bytes for device extension
							  &usDriverName,
							  FILE_DEVICE_UNKNOWN, // driver is not associated with any particular device
							  FILE_DEVICE_SECURE_OPEN,
							  FALSE, &pDeviceObject);

	// Requests made to the driver are called IRP Major requests. There are also minor requests
	// which are sub-requests of these and can be found in the stack location of the IRP.

	for(uiIndex = 0; uiIndex < IRP_MJ_MAXIMUM_FUNCTION; uiIndex++) {
		pDriverObject->MajorFunction[uiIndex] = Example_UnSupportedFunction;
	}

	pDriverObject->MajorFunction[IRP_MJ_CLOSE]			=		Example_Close;
	pDriverObject->MajorFunction[IRP_MJ_CREATE]			=		Example_Create;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]	=		Example_IoControl;
	pDriverObject->MajorFunction[IRP_MJ_READ]			=		USE_READ_FUNCTION;
	pDriverObject->MajorFunction[IRP_MJ_WRITE]			=		USE_WRITE_FUNCTION;

	// Certain APIs call directly to the driver and pass in parameters
	// CreateFile -> IRP_MJ_CREATE
	// CloseHandle -> IRP_MJ_CLEANUP & IRP_MJ_CLOSE
	// WriteFile -> IRP_MJ_WRITE
	// ReadFile -> IRP_MJ_READ
	// DeviceIoControl -> IRP_MJ_DEVICE_CONTROL
	// 
	// If you need to perform process related clean up,
	// then you need to handle IRP_MJ_CLEANUP as well.

	pDriverObject->DriverUnload = Example_Unload;

	pDeviceObject->Flags |= IO_TYPE; // IO_TYPE defines the type of I/O we want to do
	pDeviceObject->Flags &= (~DO_DEVICE_INITIALISING);
	// tells the I/O Manager that the device is being initialized
	// and not to send any I/O requests to the driver. For devices 
	// created in the context of the "DriverEntry", this is not
	// needed since the I/O Manager will clear this flag once the
	// "DriverEntry" is done. However, if a device is created in any
	// function outside of "DriverEntry", one needs to manually clear
	// this flag for any device you create with IoCreateDevice.
	// This flag is actually set by IoCreateDevice function. We cleared
	// here just for fun even though we weren't required to.

	IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);
	// creates a symbolic link in the object manager
	// A symbolic link maps a "DOS Device Name" to an
	// NT Device Name

	// Different vendors have different drivers and each driver
	// is required to have its own name. We cannot have two
	// drivers with the same NT Device name.
	// E: is a symbolic link. The network mapped drive may may
	// map to \Device\NetworkRedirector and the memory stick may
	// map to \Deivce\FujiMemoryStick, for example.

	// This is how applications can be written using a commonly
	// defined name which can be abstracted to point to any device
	// driver which would be able to handle requests.
}