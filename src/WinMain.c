#ifdef MARKVIEW_WINDOWS


#include <stdio.h>
#include <windows.h>
#include <stdlib.h>


int main(int argc, char** argv);

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{
	int argc;
	LPWSTR *argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
	char **argv = (char **)malloc(sizeof(char *) * argc);

	for (int i = 0; i < argc; i++) {
		int len = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
		argv[i] = (char *)malloc(len);
		WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], len, NULL, NULL);
	}

	printf("calling main from WinMain");
	int result = main(argc, argv);

	// Cleanup
	for (int i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
	LocalFree(argvW);

	return result;
}

#endif // MARKVIEW_WINDOWS
