#include <Windows.h>

int main()
{

	HANDLE handle = CreateFileW(L"\\\\.\\ActiveWindow",
		GENERIC_READ | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	char lpBuffer[20] = { 0 };
	DWORD numberOfBytesToRead = 0;
	DWORD num = 0;
	ReadFile(handle, lpBuffer, numberOfBytesToRead, &num, NULL);

	CloseHandle(handle);

	return 0;
}