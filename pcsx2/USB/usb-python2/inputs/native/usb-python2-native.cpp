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
			// TODO: Fix sticky input issue. Hold button, tab out, then tab back in. The button will be held until it's pressed again.
			auto keyBindStr = InputManager::ConvertInputBindingKeyToString(key);

			printf("Pressed button! %d %f %s\n", key.data, value, keyBindStr.c_str());

			for (auto mappedKey : mappingsByInputKey[keyBindStr]) {
				printf("\t%s %d\n", mappedKey.keybind.c_str(), mappedKey.isOneshot);

				if (value == 0)
				{
					if (!mappedKey.isOneshot)
						keyStateUpdates[mappedKey.keybind].push_back({std::chrono::steady_clock::now(), false});
				}
				else
				{
					if (!keyboardButtonIsPressed[keyBindStr])
					{
						keyStateUpdates[mappedKey.keybind].push_back({std::chrono::steady_clock::now(), true});

						if (mappedKey.isOneshot)
							keyStateUpdates[mappedKey.keybind].push_back({std::chrono::steady_clock::now(), false});
					}
				}
			}

			keyboardButtonIsPressed[keyBindStr] = value != 0;

			return InputInterceptHook::CallbackResult::ContinueProcessingEvent;
		}

		void NativeInput::LoadMapping()
		{
			uint32_t uniqueKeybindIdx = 0;

			SettingsInterface* si = Host::GetSettingsInterfaceForBindings();
			const std::vector<std::string> bind_names = PAD::GetControllerBinds(PAD::GetDefaultPadType(0));
			const std::string section = "Python2";
			const std::string type = "Python2";
			if (!buttonLabelList.empty())
			{
				for (u32 bind_index = 0; bind_index < static_cast<u32>(buttonLabelList.size()); bind_index++)
				{
					const std::string& bind_name = buttonLabelList[bind_index];
					const std::vector<std::string> bindings(si->GetStringList(section.c_str(), bind_name.c_str()));

					printf("section: %s, bind_name: %s\n", section.c_str(), bind_name.c_str());

					for (auto bind : bindings) {
						int isOneshot = 0;

						auto idx = bind.find_first_of(L'|');
						if (idx != std::string::npos) {
							auto substr = std::string(bind.begin() + idx + 1, bind.end());
							sscanf(substr.c_str(), "%d", &isOneshot);
						}

						auto input_key = std::string(bind.begin(), bind.begin() + idx);

						KeyMapping keybindMapping = {
							uniqueKeybindIdx++,
							input_key,
							bind_name,
							isOneshot == 1};

						mappingsByInputKey[input_key].push_back(keybindMapping);
						mappingsByButtonLabel[bind_name].push_back(keybindMapping);

						printf("\tbind: %s, oneshot = %d\n", input_key.c_str(), isOneshot);
					}
				}
			}
		}

		int NativeInput::Open()
		{
			Close();

			mappingsByInputKey.clear();
			mappingsByButtonLabel.clear();

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
				const std::chrono::duration<double, std::milli> timestampDiff = currentTimestamp - curState.timestamp;
				if (timestampDiff.count() > 150)
				{
					// Console.WriteLn("Dropping delayed input... %s %ld ms late", keybind.c_str(), timestampDiff.count());
					continue;
				}

				// Console.WriteLn("Keystate update %s %d", keybind.c_str(), curState.state);

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

		bool NativeInput::IsAnalogKeybindAvailable(std::string keybind)
		{
			return currentInputStateAnalog.find(keybind) != currentInputStateAnalog.end();
		}
	} // namespace native
} // namespace usb_python2
