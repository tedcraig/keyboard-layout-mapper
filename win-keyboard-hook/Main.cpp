#include <Windows.h>
#include <iostream>

LRESULT CALLBACK KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *eventInfo = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

	switch (wParam) {
		case WM_KEYDOWN:
			DWORD scanCode = eventInfo->scanCode;
			DWORD virtualKeyCode = eventInfo->vkCode;
			char character = MapVirtualKey(virtualKeyCode, MAPVK_VK_TO_CHAR);
			// could use SendInput() here
			std::cout << "scan code: " << scanCode << " | virtual key code: " << virtualKeyCode << " | char: " << character << std::endl;
			break;

	}

	// tell keyboard processing to keep going
	// if we want to remap key code, this is where we can pass along alternate values
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
	std::cout << "Hello World!" << std::endl;

	HHOOK keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardHookCallback, 0, 0);

	// win32 message pump
	MSG message;
	while (GetMessage(&message, NULL, NULL, NULL) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	// unhook
	UnhookWindowsHookEx(keyboard);

	return 0;
}