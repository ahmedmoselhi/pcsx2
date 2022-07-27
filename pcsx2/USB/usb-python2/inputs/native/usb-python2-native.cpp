#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "usb-python2-native.h"

#include "pcsx2/HostSettings.h"
#include "common/SettingsInterface.h"

#include "common/StringUtil.h"
#include "PAD/Host/PAD.h"

#include "USB/usb-python2/inputs/python2-inputs.h"

namespace usb_python2
{
	namespace native
	{
		InputInterceptHook::CallbackResult NativeInput::ParseInput(InputBindingKey key, float value)
		{
			// TODO: Fix sticky input issue. Hold button, tab out, then tab back in. The button will be held until it's pressed again.

			std::map<std::string, int> updatedInputState;
			auto keyBindStr = InputManager::ConvertInputBindingKeyToString(key);

			if (key.source_subtype != InputSubclass::PointerAxis && key.source_subtype != InputSubclass::ControllerAxis)
				printf("Pressed button! %d %f %s\n", key.data, value, keyBindStr.c_str());

			for (auto mappedKey : mappingsByInputKey[keyBindStr])
			{
				printf("\t%s %d\n", mappedKey.keybind.c_str(), mappedKey.isOneshot);

				if (key.source_type == InputSourceType::Keyboard)
				{
					if (value == 0)
					{
						if (updatedInputState.find(mappedKey.keybind) == updatedInputState.end() || updatedInputState[mappedKey.keybind] == 0) // Only reset value if it wasn't set by a button already
							updatedInputState[mappedKey.keybind] = keyboardButtonIsPressed[keyBindStr] ? 2 : 0;
					}
					else
					{
						if (!keyboardButtonIsPressed[keyBindStr])
							updatedInputState[mappedKey.keybind] = 1 | (mappedKey.isOneshot ? 0x80 : 0);
					}
				}
				else if (key.source_subtype == InputSubclass::ControllerAxis || key.source_subtype == InputSubclass::PointerAxis)
				{
					currentInputStateAnalog[mappedKey.keybind] = value;
				}
				else if (key.source_subtype == InputSubclass::ControllerButton || key.source_subtype == InputSubclass::PointerButton)
				{
					if (value == 0)
					{
						if (updatedInputState.find(mappedKey.keybind) == updatedInputState.end() || updatedInputState[mappedKey.keybind] == 0) // Only reset value if it wasn't set by a button already
							updatedInputState[mappedKey.keybind] = gamepadButtonIsPressed[key.data | ((int)key.source_subtype << 28)] ? 2 : 0;
					}
					else
					{
						if (!gamepadButtonIsPressed[key.data | ((int)key.source_subtype << 28)])
							updatedInputState[mappedKey.keybind] = 1 | (mappedKey.isOneshot ? 0x80 : 0);
					}
				}
			}

			if (key.source_type == InputSourceType::Keyboard)
			{
				keyboardButtonIsPressed[keyBindStr] = value != 0;
			}
			else if (key.source_subtype == InputSubclass::ControllerButton || key.source_subtype == InputSubclass::PointerButton)
			{
				gamepadButtonIsPressed[key.data | ((int)key.source_subtype << 28)] = value != 0;
			}

			for (auto& k : updatedInputState)
			{
				currentInputStatePad[k.first] = k.second;

				if ((k.second & 3) == 1)
				{
					keyStateUpdates[k.first].push_back({std::chrono::steady_clock::now(), true});

					if (k.second & 0x80)
					{
						// Oneshot
						keyStateUpdates[k.first].push_back({std::chrono::steady_clock::now(), false});
					}
				}
				else if ((k.second & 3) == 2)
				{
					keyStateUpdates[k.first].push_back({std::chrono::steady_clock::now(), false});
				}
			}

			return InputInterceptHook::CallbackResult::ContinueProcessingEvent;
		}

		void NativeInput::LoadMapping()
		{
			SettingsInterface* si = Host::GetSettingsInterfaceForBindings();
			const std::string section = "Python2";

			if (!buttonLabelList.empty())
			{
				for (u32 bind_index = 0; bind_index < static_cast<u32>(buttonLabelList.size()); bind_index++)
				{
					const std::string& bind_name = buttonLabelList[bind_index];
					const std::vector<std::string> bindings(si->GetStringList(section.c_str(), bind_name.c_str()));

					printf("button: %s\n", bind_name.c_str());

					for (auto system_entry : s_python2_system_info)
					{
						if (system_entry.bindings == nullptr)
							continue;

						for (u32 i = 0; i < system_entry.num_bindings; i++)
						{
							auto entry = system_entry.bindings[i];

							if (std::string(entry.name) != bind_name)
								continue;

							for (auto bind : bindings)
							{
								int isOneshot = 0;
								double analogDeadzone = 0;
								double analogSensitivity = 0;
								double motorScale = 0;

								auto idx = bind.find_first_of(L'|');
								if (idx != std::string::npos)
								{
									auto substr = std::string(bind.begin() + idx + 1, bind.end());

									if (entry.type == PAD::ControllerBindingType::Button)
									{
										isOneshot = std::stoi(substr);
									}
									else if (entry.type == PAD::ControllerBindingType::Axis || entry.type == PAD::ControllerBindingType::HalfAxis)
									{
										analogDeadzone = std::stod(substr);
										analogSensitivity = std::stod(substr.substr(substr.find_first_of('|') + 1));
									}
									else if (entry.type == PAD::ControllerBindingType::Motor)
									{
										motorScale = std::stod(substr);
									}
								}

								auto input_key = std::string(bind.begin(), bind.begin() + idx);

								KeyMapping keybindMapping = {
									input_key,
									bind_name,
									analogDeadzone,
									analogSensitivity,
									motorScale,
									isOneshot == 1};

								mappingsByInputKey[input_key].push_back(keybindMapping);
								mappingsByButtonLabel[bind_name].push_back(keybindMapping);

								printf("\tbind: %s, oneshot = %d\n", input_key.c_str(), isOneshot);
							}
						}
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
				if (timestampDiff.count() > 50)
				{
					Console.WriteLn("Dropping delayed input... %s %ld ms late", keybind.c_str(), timestampDiff.count());
					continue;
				}

				Console.WriteLn("Keystate update %s %d", keybind.c_str(), curState.state);
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
