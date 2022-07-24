#ifndef USBPYTHON2NATIVE_H
#define USBPYTHON2NATIVE_H
#include <thread>
#include <array>
#include <atomic>
#include <chrono>
#include "Frontend/InputManager.h"
#include "USB/usb-python2/python2proxy.h"
#include "USB/usb-python2/usb-python2.h"

namespace usb_python2
{
	namespace native
	{
		static const char* APINAME = "native";

		struct InputStateUpdate
		{
			std::chrono::steady_clock::time_point timestamp;
			bool state;
		};

		const std::vector<std::wstring> axisLabelList = {
			L"X",
			L"Y",
			L"Z",
			L"RX",
			L"RY",
			L"RZ"};

		const std::vector<std::wstring> buttonDefaultOneshotList = {
			L"DmHihat",
			L"DmSnare",
			L"DmHighTom",
			L"DmLowTom",
			L"DmCymbal",
			L"DmBassDrum",

			L"GfP1Pick",
			L"GfP2Pick"};

		const std::vector<std::wstring> buttonLabelList = {
			// Machine
			L"Test",
			L"Service",
			L"Coin1",
			L"Coin2",

			// Guitar Freaks
			L"GfP1Start",
			L"GfP1Pick",
			L"GfP1Wail",
			L"GfP1EffectInc",
			L"GfP1EffectDec",
			L"GfP1NeckR",
			L"GfP1NeckG",
			L"GfP1NeckB",

			L"GfP2Start",
			L"GfP2Pick",
			L"GfP2Wail",
			L"GfP2EffectInc",
			L"GfP2EffectDec",
			L"GfP2NeckR",
			L"GfP2NeckG",
			L"GfP2NeckB",

			// Drummania
			L"DmStart",
			L"DmSelectL",
			L"DmSelectR",
			L"DmHihat",
			L"DmSnare",
			L"DmHighTom",
			L"DmLowTom",
			L"DmCymbal",
			L"DmBassDrum",

			// DDR
			L"DdrP1Start",
			L"DdrP1SelectL",
			L"DdrP1SelectR",
			L"DdrP1FootLeft",
			L"DdrP1FootDown",
			L"DdrP1FootUp",
			L"DdrP1FootRight",

			L"DdrP2Start",
			L"DdrP2SelectL",
			L"DdrP2SelectR",
			L"DdrP2FootLeft",
			L"DdrP2FootDown",
			L"DdrP2FootUp",
			L"DdrP2FootRight",

			// Thrill Drive
			L"ThrillDriveStart",
			L"ThrillDriveGearUp",
			L"ThrillDriveGearDown",
			L"ThrillDriveWheelAnalog",
			L"ThrillDriveWheelLeft",
			L"ThrillDriveWheelRight",
			L"ThrillDriveAccelAnalog",
			L"ThrillDriveAccel",
			L"ThrillDriveBrake",
			L"ThrillDriveBrakeAnalog",
			L"ThrillDriveSeatbelt",

			// Toy's March
			L"ToysMarchP1Start",
			L"ToysMarchP1SelectL",
			L"ToysMarchP1SelectR",
			L"ToysMarchP1DrumL",
			L"ToysMarchP1DrumR",
			L"ToysMarchP1Cymbal",

			L"ToysMarchP2Start",
			L"ToysMarchP2SelectL",
			L"ToysMarchP2SelectR",
			L"ToysMarchP2DrumL",
			L"ToysMarchP2DrumR",
			L"ToysMarchP2Cymbal",

			// Dance 86.4
			L"Dance864P1Start",
			L"Dance864P1Left",
			L"Dance864P1Right",
			L"Dance864P1PadLeft",
			L"Dance864P1PadCenter",
			L"Dance864P1PadRight",

			L"Dance864P2Start",
			L"Dance864P2Left",
			L"Dance864P2Right",
			L"Dance864P2PadLeft",
			L"Dance864P2PadCenter",
			L"Dance864P2PadRight",

			// ICCA Card Reader
			L"KeypadP1_0",
			L"KeypadP1_1",
			L"KeypadP1_2",
			L"KeypadP1_3",
			L"KeypadP1_4",
			L"KeypadP1_5",
			L"KeypadP1_6",
			L"KeypadP1_7",
			L"KeypadP1_8",
			L"KeypadP1_9",
			L"KeypadP1_00",
			L"KeypadP1InsertEject",
			L"KeypadP2_0",
			L"KeypadP2_1",
			L"KeypadP2_2",
			L"KeypadP2_3",
			L"KeypadP2_4",
			L"KeypadP2_5",
			L"KeypadP2_6",
			L"KeypadP2_7",
			L"KeypadP2_8",
			L"KeypadP2_9",
			L"KeypadP2_00",
			L"KeypadP2InsertEject",
		};

		class NativeInput : public Python2Input
		{
		public:
			NativeInput(int port, const char* dev_type)
				: Python2Input(port, dev_type)
			{
			}
			~NativeInput()
			{
				Close();
			}
			int Open() override;
			int Close() override;
			int ReadPacket(std::vector<uint8_t>& data) override { return 0; }
			int WritePacket(const std::vector<uint8_t>& data) override { return 0; }
			void ReadIo(std::vector<uint8_t>& data) override {}
			int Reset() override { return 0; }

			bool isPassthrough() override { return false; }

			InputInterceptHook::CallbackResult ParseInput(InputBindingKey key, float value);

			static const TCHAR* Name()
			{
				return TEXT("Raw Input");
			}

			void UpdateKeyStates(std::wstring keybind) override;
			bool GetKeyState(std::wstring keybind) override;
			bool GetKeyStateOneShot(std::wstring keybind) override;
			double GetKeyStateAnalog(std::wstring keybind) override;
			bool IsKeybindAvailable(std::wstring keybind) override;
			bool IsAnalogKeybindAvailable(std::wstring keybind) override;

			static int Configure(int port, const char* dev_type, void* data) { return 0; }

		protected:
		private:
		};

		enum KeybindType
		{
			KeybindType_Button = 0,
			KeybindType_Axis,
			KeybindType_Hat,
			KeybindType_Keyboard
		};

		struct KeyMapping
		{
			uint32_t uniqueId;
			uint32_t keybindId;
			uint32_t bindType; // 0 = button, 1 = axis, 2 = hat, 3 = keyboard
			uint32_t value;
			bool isOneshot; // Immediately trigger an off after on
		};

		struct Mappings
		{
			std::vector<KeyMapping> mappings;

			std::wstring devName;
#if _WIN32
			std::wstring hidPath;
#endif
		};

		typedef std::vector<Mappings> MapVector;
		static MapVector mapVector;
		static std::map<int, Mappings*> mappings;

		static std::map<std::wstring, std::deque<InputStateUpdate>> keyStateUpdates;
		static std::map<std::wstring, bool> isOneshotState;
		static std::map<std::wstring, bool> currentKeyStates;
		static std::map<std::wstring, int> currentInputStateKeyboard;
		static std::map<std::wstring, int> currentInputStatePad;
		static std::map<std::wstring, double> currentInputStateAnalog;

		static std::map<uint16_t, bool> keyboardButtonIsPressed;
		static std::map<std::wstring, std::map<uint32_t, bool>> gamepadButtonIsPressed;

		void LoadMappings(const char* dev_type, MapVector& maps);
		void SaveMappings(const char* dev_type, MapVector& maps);

	} // namespace native
} // namespace usb_python2
#endif
