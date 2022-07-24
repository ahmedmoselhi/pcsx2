#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "usb-python2-native.h"

#include "pcsx2/HostSettings.h"
#include "common/SettingsInterface.h"

namespace usb_python2
{
	namespace native
	{
		std::wstring getKeyLabel(const KeyMapping key);

		uint32_t axisDiff2[32]; //previous axes values
		bool axisPass22 = false;

		static bool resetKeybinds = false;

		InputInterceptHook::CallbackResult NativeInput::ParseInput(InputBindingKey key, float value)
		{
			printf("Pressed button! %d %s\n", key.data, InputManager::ConvertInputBindingKeyToString(key).c_str());
			return InputInterceptHook::CallbackResult::ContinueProcessingEvent;
		}

		int NativeInput::Open()
		{
			Close();

			mappings.clear();
			currentInputStateKeyboard.clear();
			currentInputStatePad.clear();
			currentInputStateAnalog.clear();
			currentKeyStates.clear();
			keyStateUpdates.clear();

			keyboardButtonIsPressed.clear();
			gamepadButtonIsPressed.clear();

			// TODO: Load bindings from INI

			InputManager::SetHook([this](InputBindingKey key, float value) {
				return this->ParseInput(key, value);
			});

			return 0;
		}

		int NativeInput::Close()
		{
			InputManager::RemoveHook();
			return 0;
		}

		void NativeInput::UpdateKeyStates(std::wstring keybind)
		{
			const auto currentTimestamp = std::chrono::steady_clock::now();
			while (keyStateUpdates[keybind].size() > 0)
			{
				auto curState = keyStateUpdates[keybind].front();
				keyStateUpdates[keybind].pop_front();

				// Remove stale inputs that occur during times where the game can't query for inputs.
				// The timeout value is based on what felt ok to me so just adjust as needed.
				const std::chrono::duration<double, std::micro> timestampDiff = currentTimestamp - curState.timestamp;
				if (timestampDiff.count() > 150)
				{
					//Console.WriteLn(L"Dropping delayed input... %s %ld ms late", keybind, timestampDiff.count());
					continue;
				}

				//Console.WriteLn(L"Keystate update %s %d", keybind, curState.state);

				currentKeyStates[keybind] = curState.state;

				break;
			}
		}

		bool NativeInput::GetKeyState(std::wstring keybind)
		{
			UpdateKeyStates(keybind);

			auto it = currentKeyStates.find(keybind);
			if (it != currentKeyStates.end())
				return it->second;

			return false;
		}

		bool NativeInput::GetKeyStateOneShot(std::wstring keybind)
		{
			UpdateKeyStates(keybind);

			auto isPressed = false;
			auto it = currentKeyStates.find(keybind);
			if (it != currentKeyStates.end())
			{
				isPressed = it->second;
				it->second = false;
			}

			return isPressed;
		}

		double NativeInput::GetKeyStateAnalog(std::wstring keybind)
		{
			const auto it = currentInputStateAnalog.find(keybind);
			if (it == currentInputStateAnalog.end())
				return 0;
			return it->second;
		}

		bool NativeInput::IsKeybindAvailable(std::wstring keybind)
		{
			return currentInputStateKeyboard.find(keybind) != currentInputStateKeyboard.end() || currentInputStatePad.find(keybind) != currentInputStatePad.end();
		}

		bool NativeInput::IsAnalogKeybindAvailable(std::wstring keybind)
		{
			return currentInputStateAnalog.find(keybind) != currentInputStateAnalog.end();
		}
	} // namespace native
} // namespace usb_python2
