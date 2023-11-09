#include <Windows.h>
#include <iostream>
#include <vector>

HHOOK keyboardHook;

LRESULT CALLBACK remapKeyboard(int nCode, WPARAM wParam, LPARAM lParam)
{
	
	/*
		typedef struct tagKBDLLHOOKSTRUCT {
		  DWORD     vkCode;
		  DWORD     scanCode;
		  DWORD     flags;
		  DWORD     time;
		  ULONG_PTR dwExtraInfo;
		} KBDLLHOOKSTRUCT, *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;
	*/
	KBDLLHOOKSTRUCT *pEventInfo = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

	DWORD incomingScanCode = pEventInfo->scanCode;
	DWORD incomingVirtualKeyCode = pEventInfo->vkCode;
	char character;

	BOOLEAN bRemapKey = false;
	BOOLEAN bTryMutatingLParam = false;

	DWORD remappedVKCode = 0;

	UINT responseFromSend;

	std::vector<INPUT> vInputsToSend;

	character = MapVirtualKey(incomingVirtualKeyCode, MAPVK_VK_TO_CHAR);
	std::cout << "scan code: " << incomingScanCode << " | raw vk code: " << incomingVirtualKeyCode << " | raw char: " << character << std::endl;

	switch (pEventInfo->vkCode) {
		case 70: // 'F'
			
			bRemapKey = true;
			
			INPUT inputs[1];
			
			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wScan = 0;
			inputs[0].ki.dwFlags = 0;
			inputs[0].ki.time = 0;
			inputs[0].ki.dwExtraInfo = 0;

			if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
				inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
			}

			// if the incoming event is not an injected event, go ahead and remap
			if ( (pEventInfo->flags & LLKHF_INJECTED) == 0) {
				inputs[0].ki.wVk = 84; // 'T'
				responseFromSend = SendInput(1, inputs, sizeof(INPUT));
				return 1;
			}
		
			break; // end of 'F'

	}

	if (bRemapKey) {

		int numInputs = vInputsToSend.size();
		character = MapVirtualKey(incomingVirtualKeyCode, MAPVK_VK_TO_CHAR);
		std::cout << " +-- remapping " << character << " to " << std::endl;

		std::vector<INPUT>::iterator iter = vInputsToSend.begin();

		for (iter; iter < vInputsToSend.end(); iter++) {
			character = MapVirtualKey(iter->ki.wVk, MAPVK_VK_TO_CHAR);
			std::cout << "    `--> " << character << std::endl;
			
		}

		// convert from vector to array in order to avoid error converting from vector<INPUT> to LPINPUT
		INPUT* daInputsToSend = new INPUT[numInputs];

		std::copy(vInputsToSend.begin(), vInputsToSend.end(), daInputsToSend);

		// experimental early return
		if (bTryMutatingLParam) {
			// DWORD *vkCodeAddress = &eventInfo->vkCode;

			std::cout << "  experimental return..." << std::endl;
			std::cout << "    orig eventIngo VKC: " << pEventInfo->vkCode << std::endl;
			
			pEventInfo->vkCode = remappedVKCode;
			
			std::cout << "    new  eventInfo VKC: " << pEventInfo->vkCode << std::endl;
			
			lParam = LPARAM(pEventInfo);
			KBDLLHOOKSTRUCT* newEventInfo = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

			std::cout << "    new  lParam    VKC: " << newEventInfo->vkCode << std::endl;
			
			return CallNextHookEx(NULL, nCode, wParam, lParam);
		}

		UINT uSent = SendInput(numInputs, daInputsToSend, sizeof(INPUT));

		if (uSent != numInputs) {
			std::cout << "SendInput failed" << std::endl;
		}

		// skip calling the next hook and instead return a negative value
		return 1;
	}

	std::cout << "calling next hook ..." << std::endl;
	
	// if we get here then there was no remapping
	// so pass the hook on in order to keep processing
	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

int main()
{
	std::cout << "Keyboard Layout Mapper" << std::endl;
	std::cout << "running..." << std::endl;

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, &remapKeyboard, 0, 0);

	// win32 message pump
	MSG message;
	while (GetMessage(&message, NULL, NULL, NULL) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	// cleanup by removing the hook
	UnhookWindowsHookEx(keyboardHook);

	return 0;
}