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
			int Reset() override;

			bool isPassthrough() override { return false; }

			InputInterceptHook::CallbackResult ParseInput(InputBindingKey key, float value);

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
			KeybindType_Keyboard,
			KeybindType_Motor,
		};

		struct KeyMapping
		{
			std::string inputKey;
			std::string keybind;
			double analogDeadzone;
			double analogSensitivity;
			double motorScale;
			bool isOneshot; // Immediately trigger an off after on
		};

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
