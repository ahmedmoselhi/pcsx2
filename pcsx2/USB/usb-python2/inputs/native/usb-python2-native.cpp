#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "usb-python2-native.h"

#include "pcsx2/HostSettings.h"
#include "common/SettingsInterface.h"

#include "common/StringUtil.h"
#include "PAD/Host/PAD.h"

namespace usb_python2
{
	namespace native
	{
		std::string getKeyLabel(const KeyMapping key);

		uint32_t axisDiff2[32]; //previous axes values
		bool axisPass22 = false;

		static bool resetKeybinds = false;

		InputInterceptHook::CallbackResult NativeInput::ParseInput(InputBindingKey key, float value)
		{
			printf("Pressed button! %d %s\n", key.data, InputManager::ConvertInputBindingKeyToString(key).c_str());
			return InputInterceptHook::CallbackResult::ContinueProcessingEvent;
		}

		void NativeInput::LoadMapping()
		{
			uint32_t uniqueKeybindIdx = 0;

			Mappings m;

			SettingsInterface* si = Host::GetSettingsInterfaceForBindings();
			const std::vector<std::string> bind_names = PAD::GetControllerBinds(PAD::GetDefaultPadType(0));
			const std::string section(StringUtil::StdStringFromFormat("Pad%u", 0 + 1));
			const std::string type(si->GetStringValue(section.c_str(), "Type", PAD::GetDefaultPadType(0)));
			if (!bind_names.empty())
			{
				for (u32 bind_index = 0; bind_index < static_cast<u32>(bind_names.size()); bind_index++)
				{
					const std::string& bind_name = bind_names[bind_index];
					const std::vector<std::string> bindings(si->GetStringList(section.c_str(), bind_name.c_str()));

					printf("section: %s, bind_name: %s\n", section.c_str(), bind_name.c_str());

					for (auto bind : bindings) {
						uint32_t keybindId = 0;
						uint32_t buttonType = 0;
						uint32_t value = 0;
						bool isOneshot = false;

						KeyMapping keybindMapping = {
							uniqueKeybindIdx++,
							keybindId,
							buttonType,
							value,
							isOneshot == 1};
						m.mappings.push_back(keybindMapping);

						printf("\tbind: %s\n", bind.c_str());
					}
				}
			}
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

			LoadMapping();

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

		void NativeInput::UpdateKeyStates(std::string keybind)
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
					//Console.WriteLn("Dropping delayed input... %s %ld ms late", keybind, timestampDiff.count());
					continue;
				}

				//Console.WriteLn("Keystate update %s %d", keybind, curState.state);

				currentKeyStates[keybind] = curState.state;

				break;
			}
		}

		bool NativeInput::GetKeyState(std::string keybind)
		{
			UpdateKeyStates(keybind);

			auto it = currentKeyStates.find(keybind);
			if (it != currentKeyStates.end())
				return it->second;

			return false;
		}

		bool NativeInput::GetKeyStateOneShot(std::string keybind)
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

		double NativeInput::GetKeyStateAnalog(std::string keybind)
		{
			const auto it = currentInputStateAnalog.find(keybind);
			if (it == currentInputStateAnalog.end())
				return 0;
			return it->second;
		}

		bool NativeInput::IsKeybindAvailable(std::string keybind)
		{
			return currentInputStateKeyboard.find(keybind) != currentInputStateKeyboard.end() || currentInputStatePad.find(keybind) != currentInputStatePad.end();
		}

		bool NativeInput::IsAnalogKeybindAvailable(std::string keybind)
		{
			return currentInputStateAnalog.find(keybind) != currentInputStateAnalog.end();
		}
	} // namespace native
} // namespace usb_python2
