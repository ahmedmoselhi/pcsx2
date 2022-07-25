#ifndef USBPYTHON2RAW_H
#define USBPYTHON2RAW_H
#include <thread>
#include <array>
#include <atomic>
#include "USB/usb-python2/python2proxy.h"
#include "USB/usb-python2/usb-python2.h"
#include "USB/shared/rawinput_usb.h"

namespace usb_python2
{
#define CHECK(exp) \
	do \
	{ \
		if (!(exp)) \
			goto Error; \
	} while (0)
#define SAFE_FREE(p) \
	do \
	{ \
		if (p) \
		{ \
			free(p); \
			(p) = NULL; \
		} \
	} while (0)

	namespace raw
	{
		struct InputStateUpdate
		{
			wxDateTime timestamp;
			bool state;
		};

		const std::vector<char*> axisLabelList = {
			"X",
			"Y",
			"Z",
			"RX",
			"RY",
			"RZ"};

		const std::vector<char*> buttonDefaultOneshotList = {
			"DmHihat",
			"DmSnare",
			"DmHighTom",
			"DmLowTom",
			"DmCymbal",
			"DmBassDrum",

			"GfP1Pick",
			"GfP2Pick"};

		const std::vector<char*> buttonLabelList = {
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
			"DmSelectL",
			"DmSelectR",
			"DmHihat",
			"DmSnare",
			"DmHighTom",
			"DmLowTom",
			"DmCymbal",
			"DmBassDrum",

			// DDR
			"DdrP1Start",
			"DdrP1SelectL",
			"DdrP1SelectR",
			"DdrP1FootLeft",
			"DdrP1FootDown",
			"DdrP1FootUp",
			"DdrP1FootRight",

			"DdrP2Start",
			"DdrP2SelectL",
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
			"ToysMarchP1SelectL",
			"ToysMarchP1SelectR",
			"ToysMarchP1DrumL",
			"ToysMarchP1DrumR",
			"ToysMarchP1Cymbal",

			"ToysMarchP2Start",
			"ToysMarchP2SelectL",
			"ToysMarchP2SelectR",
			"ToysMarchP2DrumL",
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

		class RawInputPad : public Python2Input, shared::rawinput::ParseRawInputCB
		{
		public:
			RawInputPad(int port, const char* dev_type)
				: Python2Input(port, dev_type)
			{
				if (!InitHid())
					throw UsbPython2Error("InitHid() failed!");
			}
			~RawInputPad()
			{
				Close();
			}
			int Open() override;
			int Close() override;
			int ReadPacket(std::vector<uint8_t>& data) override { return 0; };
			int WritePacket(const std::vector<uint8_t>& data) override { return 0; };
			void ReadIo(std::vector<uint8_t>& data) override {}
			int Reset() override { return 0; }
			void ParseRawInput(PRAWINPUT pRawInput);

			bool isPassthrough() override { return false; }

			static const TCHAR* Name()
			{
				return TEXT("Raw Input");
			}

			void UpdateKeyStates(std::string keybind) override;
			bool GetKeyState(std::string keybind) override;
			bool GetKeyStateOneShot(std::string keybind) override;
			double GetKeyStateAnalog(std::string keybind) override;
			bool IsAnalogKeybindAvailable(std::string keybind) override;

			static int Configure(int port, const char* dev_type, void* data);

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

			std::string devName;
#if _WIN32
			std::string hidPath;
#endif
		};

#ifndef PCSX2_CORE
		struct Python2DlgConfig
		{
			int port;
			const char* dev_type;

			const std::vector<std::string> devList;
			const std::vector<std::string> devListGroups;

			Python2DlgConfig(int p, const char* dev_type_, const std::vector<std::string>& devList, const std::vector<std::string>& devListGroups)
				: port(p)
				, dev_type(dev_type_)
				, devList(devList)
				, devListGroups(devListGroups)
			{
			}
		};
#endif

		typedef std::vector<Mappings> MapVector;
		static MapVector mapVector;
		static std::map<HANDLE, Mappings*> mappings;

		static std::map<std::string, std::deque<InputStateUpdate>> keyStateUpdates;
		static std::map<std::string, bool> isOneshotState;
		static std::map<std::string, bool> currentKeyStates;
		static std::map<std::string, double> currentInputStateAnalog;

		static std::map<uint16_t, bool> keyboardButtonIsPressed;
		static std::map<std::string, std::map<uint32_t, bool>> gamepadButtonIsPressed;

		void LoadMappings(const char* dev_type, MapVector& maps);
		void SaveMappings(const char* dev_type, MapVector& maps);

	} // namespace raw
} // namespace usb_python2
#endif
