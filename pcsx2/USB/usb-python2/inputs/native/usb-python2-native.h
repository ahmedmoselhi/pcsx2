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

		const std::vector<std::string> axisLabelList = {
			"X",
			"Y",
			"Z",
			"RX",
			"RY",
			"RZ"};

		const std::vector<std::string> buttonDefaultOneshotList = {
			"DmHihat",
			"DmSnare",
			"DmHighTom",
			"DmLowTom",
			"DmCymbal",
			"DmBassDrum",

			"GfP1Pick",
			"GfP2Pick"};

		const std::vector<std::string> buttonLabelList = {
			// Machine
			"Test",
			"Service",
			"Coin1",
			"Coin2",

			// Guitar Freaks
			"GfP1Start",
			"GfP1Pick",
			"GfP1Wail",
			"GfP1EffectInc",
			"GfP1EffectDec",
			"GfP1NeckR",
			"GfP1NeckG",
			"GfP1NeckB",

			"GfP2Start",
			"GfP2Pick",
			"GfP2Wail",
			"GfP2EffectInc",
			"GfP2EffectDec",
			"GfP2NeckR",
			"GfP2NeckG",
			"GfP2NeckB",

			// Drummania
			"DmStart",
			"DmSelect",
			"DmSelectR",
			"DmHihat",
			"DmSnare",
			"DmHighTom",
			"DmLowTom",
			"DmCymbal",
			"DmBassDrum",

			// DDR
			"DdrP1Start",
			"DdrP1Select",
			"DdrP1SelectR",
			"DdrP1FootLeft",
			"DdrP1FootDown",
			"DdrP1FootUp",
			"DdrP1FootRight",

			"DdrP2Start",
			"DdrP2Select",
			"DdrP2SelectR",
			"DdrP2FootLeft",
			"DdrP2FootDown",
			"DdrP2FootUp",
			"DdrP2FootRight",

			// Thrill Drive
			"ThrillDriveStart",
			"ThrillDriveGearUp",
			"ThrillDriveGearDown",
			"ThrillDriveWheelAnalog",
			"ThrillDriveWheelLeft",
			"ThrillDriveWheelRight",
			"ThrillDriveAccelAnalog",
			"ThrillDriveAccel",
			"ThrillDriveBrake",
			"ThrillDriveBrakeAnalog",
			"ThrillDriveSeatbelt",

			// Toy's March
			"ToysMarchP1Start",
			"ToysMarchP1Select",
			"ToysMarchP1SelectR",
			"ToysMarchP1Drum",
			"ToysMarchP1DrumR",
			"ToysMarchP1Cymbal",

			"ToysMarchP2Start",
			"ToysMarchP2Select",
			"ToysMarchP2SelectR",
			"ToysMarchP2Drum",
			"ToysMarchP2DrumR",
			"ToysMarchP2Cymbal",

			// Dance 86.4
			"Dance864P1Start",
			"Dance864P1Left",
			"Dance864P1Right",
			"Dance864P1PadLeft",
			"Dance864P1PadCenter",
			"Dance864P1PadRight",

			"Dance864P2Start",
			"Dance864P2Left",
			"Dance864P2Right",
			"Dance864P2PadLeft",
			"Dance864P2PadCenter",
			"Dance864P2PadRight",

			// ICCA Card Reader
			"KeypadP1_0",
			"KeypadP1_1",
			"KeypadP1_2",
			"KeypadP1_3",
			"KeypadP1_4",
			"KeypadP1_5",
			"KeypadP1_6",
			"KeypadP1_7",
			"KeypadP1_8",
			"KeypadP1_9",
			"KeypadP1_00",
			"KeypadP1InsertEject",
			"KeypadP2_0",
			"KeypadP2_1",
			"KeypadP2_2",
			"KeypadP2_3",
			"KeypadP2_4",
			"KeypadP2_5",
			"KeypadP2_6",
			"KeypadP2_7",
			"KeypadP2_8",
			"KeypadP2_9",
			"KeypadP2_00",
			"KeypadP2InsertEject",
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
			void LoadMapping();

			static const TCHAR* Name()
			{
				return TEXT("Raw Input");
			}

			void UpdateKeyStates(std::string keybind) override;
			bool GetKeyState(std::string keybind) override;
			bool GetKeyStateOneShot(std::string keybind) override;
			double GetKeyStateAnalog(std::string keybind) override;
			bool IsAnalogKeybindAvailable(std::string keybind) override;

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
			std::string inputKey;
			std::string keybind;
			uint32_t bindType;
			bool isOneshot; // Immediately trigger an off after on
		};

		static std::map<std::string, std::vector<KeyMapping>> mappingsByInputKey;
		static std::map<std::string, std::vector<KeyMapping>> mappingsByButtonLabel;

		static std::map<std::string, std::deque<InputStateUpdate>> keyStateUpdates;
		static std::map<std::string, bool> isOneshotState;
		static std::map<std::string, bool> currentKeyStates;
		static std::map<std::string, int> currentInputStateKeyboard;
		static std::map<std::string, int> currentInputStatePad;
		static std::map<std::string, double> currentInputStateAnalog;

		static std::map<std::string, bool> keyboardButtonIsPressed;
		static std::map<uint32_t, bool> gamepadButtonIsPressed;

	} // namespace native
} // namespace usb_python2
#endif
