#include "Combinations.h"
// ^ Remove line or create your own Combinations.h

#include <iostream>
#include <Windows.h>
#include <map>
#include <functional>
#include <chrono>

#pragma comment(lib, "winmm.lib")

#define VK_SLASH 0x2F

bool Toggled = false;
short KeyState;

typedef std::string str;

// Time since last key press
auto StartTime = std::chrono::steady_clock::now();

str CurrentCombination = "";
int CombinationCount = 0;

HKL KeyboardLayout = GetKeyboardLayout(0);

void KeyboardWrite(str Text, int Delay = 0) {
	for (char Character : Text) {

		SHORT VK = VkKeyScanEx(Character, KeyboardLayout);
		unsigned int VSC = MapVirtualKey(VK, MAPVK_VK_TO_VSC);

		// Press
		keybd_event(VK, VSC, 0, 0);

		// Release
		keybd_event(VK, VSC, KEYEVENTF_KEYUP, 0);

		if (Delay > 0) {
			Sleep(Delay);
		}
	}
}

// Single key
void KeyboardPress(int Character, int Delay = 0) {
	SHORT VK = VkKeyScanEx(Character, KeyboardLayout);
	unsigned int VSC = MapVirtualKey(VK, MAPVK_VK_TO_VSC);

	// Press
	keybd_event(VK, VSC, 0, 0);

	// Release
	keybd_event(VK, VSC, KEYEVENTF_KEYUP, 0);

	if (Delay > 0) {
		Sleep(Delay);
	}
}

// Send combination (different inputs depending on focused application)
void SendCombination(str Text) {
	wchar_t Buffer[MAX_CLASS_NAME];
	GetWindowText(GetForegroundWindow(), Buffer, MAX_CLASS_NAME);
	std::wstring WString(Buffer);

	// ---------- ROBLOX ----------
	if (WString == L"Roblox") {
		Sleep(40);
		KeyboardPress(VK_SLASH);
		Sleep(40);
		KeyboardWrite(Text);
		Sleep(40);
		KeyboardPress(VK_RETURN);
		Sleep(80);
	}
	else {
		std::cout << CombinationCount << "\n";
		for (int i = 0; i < CombinationCount; i++) {
			KeyboardPress(VK_BACK);
		}
		KeyboardWrite(Text);
		KeyboardPress(VK_RETURN);
	}
}

// Checks if the current combination is valid, and sends its text if it is.
void CheckCombination() {
	// Combinations is a map<str, str> formatted like so:
	// std::map<std::string, std::string> Combinations = { { "97 97", "hello" }, { "97 98", "hi" } };
	// The key is the combination, the value is the text to be sent from that combination
	// Numpad1 is 97, Numpad2 is 98 and so on till Numpad9 (105)

	for (auto const& [Combination, Text] : Combinations)
	{
		if (CurrentCombination == Combination) {
			std::cout << "Combination" << "\n";
			SendCombination(Text);
		}
	}
}

// Called when a combination key (Numpad 1-9) is pressed
void OnPress(int KeyCode) {
	if (CurrentCombination.size() > 0)
		CurrentCombination += " " + std::to_string(KeyCode);
	else
		CurrentCombination = std::to_string(KeyCode);

	CombinationCount++;
	StartTime = std::chrono::steady_clock::now();

	CheckCombination();
}

// Not optimal, but I can't be asked to change it :)
std::map<int, std::function<void(int)>> Keybinds = { {VK_NUMPAD1, OnPress}, {VK_NUMPAD2, OnPress}, {VK_NUMPAD3, OnPress}, {VK_NUMPAD4, OnPress}, {VK_NUMPAD5, OnPress}, {VK_NUMPAD6, OnPress}, {VK_NUMPAD7, OnPress}, {VK_NUMPAD8, OnPress}, {VK_NUMPAD9, OnPress} };

std::map<int, bool> KeyStates = {};

int main()
{
	timeBeginPeriod(1);
	// Create key state for each bind
	for (auto const& [Key, Function] : Keybinds)
	{
		KeyStates[Key] = false;
	}

	while (true) {
		auto EndTime = std::chrono::steady_clock::now();
		auto TimePassed = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime);

		if (TimePassed.count() >= 500) {
			StartTime = std::chrono::steady_clock::now();

			CurrentCombination = "";
			CombinationCount = 0;
		}

		for (auto const& [Key, Function] : Keybinds)
		{
			KeyState = GetAsyncKeyState(Key);

			//std::cout << KeyState << "\n";

			if (KeyState < 0 && KeyStates[Key] == false) {
				KeyStates[Key] = true;
				Function(Key);
			}
			else if (KeyState >= 0 && KeyStates[Key] == true) {
				KeyStates[Key] = false;
			}
		}
		Sleep(1);
	}
	timeEndPeriod(1);
}
