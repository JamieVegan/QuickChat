#include <iostream>
#include <Windows.h>
#include <map>
#include <functional>
#include <chrono>

// JSON
// License is at Libraries/nlohmann/LICENSE.MIT
#include "Libraries/nlohmann/json.hpp"
#include <fstream>
using json = nlohmann::json;

// timeBeginPeriod & timeEndPeriod
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

std::map<str, str> Combinations;

void KeyboardWrite(str Text, int Delay = 0) {
	for (char Character : Text) {

		SHORT VK = VkKeyScanEx(Character, KeyboardLayout);
		unsigned int VSC = MapVirtualKey(VK, MAPVK_VK_TO_VSC);

		SHORT VK2 = 0;
		unsigned int VSC2 = 0;

		// Shift press
		if (isupper(Character)) {
			VK2 = VK_LSHIFT;
			VSC = MapVirtualKey(VK2, MAPVK_VK_TO_VSC);
			keybd_event(VK2, VSC2, 0, 0);
		}

		// Press
		keybd_event(VK, VSC, 0, 0);

		// Release
		keybd_event(VK, VSC, KEYEVENTF_KEYUP, 0);

		// Shift release
		if (isupper(Character)) {
			keybd_event(VK2, VSC2, KEYEVENTF_KEYUP, 0);
		}

		if (Delay > 0) {
			Sleep(Delay);
		}
	}
}

// Single key
void KeyboardPress(int Character) {
	SHORT VK = VkKeyScanEx(Character, KeyboardLayout);
	unsigned int VSC = MapVirtualKey(VK, MAPVK_VK_TO_VSC);

	SHORT VK2 = 0;
	unsigned int VSC2 = 0;

	// Shift press
	if (isupper(Character)) {
		VK2 = VK_LSHIFT;
		VSC = MapVirtualKey(VK2, MAPVK_VK_TO_VSC);
		keybd_event(VK2, VSC2, 0, 0);
	}

	// Press
	keybd_event(VK, VSC, 0, 0);

	// Release
	keybd_event(VK, VSC, KEYEVENTF_KEYUP, 0);

	// Shift release
	if (isupper(Character)) {
		keybd_event(VK2, VSC2, KEYEVENTF_KEYUP, 0);
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
	else if (WString.ends_with(L"Discord")) {
		std::cout << CombinationCount << "\n";
		for (int i = 0; i < CombinationCount; i++) {
			KeyboardPress(VK_BACK);
		}
		KeyboardWrite(Text);
		KeyboardPress(VK_RETURN);
	}
	else {
		std::cout << CombinationCount << "\n";
		for (int i = 0; i < CombinationCount; i++) {
			KeyboardPress(VK_BACK);
		}
		KeyboardWrite(Text);
	}
}

// Checks if the current combination is valid, and sends its text if it is.
void CheckCombination() {
	if (Combinations[CurrentCombination] != "") {
		SendCombination(Combinations[CurrentCombination]);
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

	std::ifstream File("Combinations.json");

	try {
		json JSONCombinations = json::parse(File);
		Combinations = JSONCombinations.get<std::map<str, str>>();
	}
	catch (json::parse_error Exception) {
		std::cerr << "Failed to parse Combinations.json" << "\n";
		std::cerr << "Double check that your json is valid" << "\n";
		Sleep(4000);
		return -1;
	}

	File.close();

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
	return 0;
}
