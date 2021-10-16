/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "USB/Win32/Config_usb.h"
#include "usb-python2-raw.h"

#include <wx/fileconf.h>
#include "common/IniInterface.h"
#include "gui/AppConfig.h"

#include "DEV9/DEV9.h"

namespace usb_python2
{
	namespace raw
	{
		std::wstring getKeyLabel(const KeyMapping key);

		uint32_t axisDiff2[32]; //previous axes values
		bool axisPass22 = false;

		static bool resetKeybinds = false;

		void RawInputPad::WriterThread(void* ptr)
		{
			DWORD res = 0, res2 = 0, written = 0;
			std::array<uint8_t, 8> buf;

			RawInputPad* pad = static_cast<RawInputPad*>(ptr);
			pad->mWriterThreadIsRunning = true;

			while (pad->mUsbHandle != INVALID_HANDLE_VALUE)
			{
				if (pad->mFFData.wait_dequeue_timed(buf, std::chrono::milliseconds(1000)))
				{
					res = WriteFile(pad->mUsbHandle, buf.data(), buf.size(), &written, &pad->mOLWrite);
					uint8_t* d = buf.data();

					WaitForSingleObject(pad->mOLWrite.hEvent, 1000);
				}
			}

			pad->mWriterThreadIsRunning = false;
		}

		void RawInputPad::ReaderThread(void* ptr)
		{
			RawInputPad* pad = static_cast<RawInputPad*>(ptr);
			DWORD res = 0, res2 = 0, read = 0;
			std::array<uint8_t, 32> report; //32 is random

			pad->mReaderThreadIsRunning = true;
			int errCount = 0;

			while (pad->mUsbHandle != INVALID_HANDLE_VALUE)
			{
				if (GetOverlappedResult(pad->mUsbHandle, &pad->mOLRead, &read, FALSE)) // TODO check if previous read finally completed after WaitForSingleObject timed out
					ReadFile(pad->mUsbHandle, report.data(), std::min(pad->mCaps.InputReportByteLength, (USHORT)report.size()), nullptr, &pad->mOLRead); // Seems to only read data when input changes and not current state overall

				if (WaitForSingleObject(pad->mOLRead.hEvent, 1000) == WAIT_OBJECT_0)
				{
					if (!pad->mReportData.try_enqueue(report)) // TODO May leave queue with too stale data. Use multi-producer/consumer queue?
					{
						if (!errCount)
							Console.Warning("%s: Could not enqueue report data: %zd\n", APINAME, pad->mReportData.size_approx());
						errCount = (++errCount) % 16;
					}
				}
			}

			pad->mReaderThreadIsRunning = false;
		}

		int RawInputPad::TokenIn(uint8_t* buf, int len)
		{
			ULONG value = 0;
			int player = 1 - mPort;

			//Console.Warning("usb-pad: poll len=%li\n", len);
			if (mDoPassthrough)
			{
				std::array<uint8_t, 32> report; //32 is random
				if (mReportData.try_dequeue(report))
				{
					//ZeroMemory(buf, len);
					int size = std::min((int)mCaps.InputReportByteLength, len);
					memcpy(buf, report.data(), size);
					return size;
				}
				return 0;
			}

			// TODO

			return len;
		}

		int RawInputPad::TokenOut(const uint8_t* data, int len)
		{
			if (mUsbHandle == INVALID_HANDLE_VALUE)
				return 0;

			// TODO
			std::array<uint8_t, 8> report{0};

			//If i'm reading it correctly MOMO report size for output has Report Size(8) and Report Count(7), so that's 7 bytes
			//Now move that 7 bytes over by one and add report id of 0 (right?). Supposedly mandatory for HIDs.
			memcpy(report.data() + 1, data, report.size() - 1);

			if (!mFFData.enqueue(report))
			{
				return 0;
			}

			return len;
		}

		static void ParseRawInputHID(PRAWINPUT pRawInput)
		{
			PHIDP_PREPARSED_DATA pPreparsedData = NULL;
			HIDP_CAPS Caps = {0};
			PHIDP_BUTTON_CAPS pButtonCaps = NULL;
			PHIDP_VALUE_CAPS pValueCaps = NULL;
			UINT bufferSize = 0;
			ULONG usageLength, value;
			TCHAR name[1024] = {0};
			UINT nameSize = 1024;
			RID_DEVICE_INFO devInfo = {0};
			std::wstring devName;
			USHORT capsLength = 0;
			USAGE usage[32] = {0};
			Mappings* mapping = NULL;

			std::vector<uint32_t> usageCountButtons(countof(usage));
			std::vector<uint32_t> usageCountHats(8);
			std::map<LPWSTR, int> updatedInputState;

			auto iter = mappings.find(pRawInput->header.hDevice);
			if (iter != mappings.end())
			{
				mapping = iter->second;
			}
			else
			{
				GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_DEVICENAME, name, &nameSize);

				devName = name;
				std::transform(devName.begin(), devName.end(), devName.begin(), ::toupper);

				for (auto& it : mapVector)
				{
					if (it.hidPath == devName)
					{
						mapping = &it;
						mappings[pRawInput->header.hDevice] = mapping;
						break;
					}
				}
			}

			if (mapping == NULL)
				return;

			CHECK(GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize) == 0);
			CHECK(pPreparsedData = (PHIDP_PREPARSED_DATA)malloc(bufferSize));
			CHECK((int)GetRawInputDeviceInfo(pRawInput->header.hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) >= 0);
			CHECK(HidP_GetCaps(pPreparsedData, &Caps) == HIDP_STATUS_SUCCESS);

			//Get pressed buttons
			CHECK(pButtonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps));
			capsLength = Caps.NumberInputButtonCaps;
			HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData);

			uint32_t numberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;
			usageLength = countof(usage);

			if (HidP_GetUsages(
					 HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData,
					 (PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) == HIDP_STATUS_SUCCESS)
			{
				for (uint32_t i = 0; i < usageLength; i++)
				{
					usageCountButtons[usage[i] - pButtonCaps->Range.UsageMin + 1]++;
				}
			}

			/// Get axes' values
			CHECK(pValueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps));
			capsLength = Caps.NumberInputValueCaps;
			if (HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) == HIDP_STATUS_SUCCESS)
			{
				for (USHORT i = 0; i < capsLength; i++)
				{
					if (HidP_GetUsageValue(
							HidP_Input, pValueCaps[i].UsagePage, 0,
							pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
							(PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS)
					{
						continue; // if here then maybe something is up with HIDP_CAPS.NumberInputValueCaps
					}

					//Get mapped axis for physical axis
					uint16_t v = 0;
					switch (pValueCaps[i].Range.UsageMin)
					{
						case HID_USAGE_GENERIC_X: //0x30
						case HID_USAGE_GENERIC_Y:
						case HID_USAGE_GENERIC_Z:
						case HID_USAGE_GENERIC_RX:
						case HID_USAGE_GENERIC_RY:
						case HID_USAGE_GENERIC_RZ: //0x35
							v = pValueCaps[i].Range.UsageMin - HID_USAGE_GENERIC_X;
							for (auto& mappedKey : mapping->mappings)
							{
								if (mappedKey.bindType == KeybindType_Axis && mappedKey.value == v)
									currentInputStateAnalog[buttonLabelList[mappedKey.keybindId]] = (value - pValueCaps[i].LogicalMin) / double(pValueCaps[i].LogicalMax - pValueCaps[i].LogicalMin);
							}
							break;
						case HID_USAGE_GENERIC_HATSWITCH:
							for (int i = 0; i < usageCountHats.size(); i++)
								usageCountHats[i] = value == i;
							break;
					}

				}
			}

			for (auto& mappedKey : mapping->mappings)
			{
				// Update buttons
				for (int i = 0; i < usageCountButtons.size(); i++)
				{
					if (mappedKey.bindType == KeybindType_Button && mappedKey.value == i)
					{
						if (usageCountButtons[mappedKey.value] > 0)
							updatedInputState[buttonLabelList[mappedKey.keybindId]] = 1;
						else if (updatedInputState.find(buttonLabelList[mappedKey.keybindId]) == updatedInputState.end()) // Only reset value if it wasn't set by a button already
							updatedInputState[buttonLabelList[mappedKey.keybindId]] = 0;
					}
				}

				// Update hats
				for (int i = 0; i < usageCountHats.size(); i++)
				{
					if (mappedKey.bindType == KeybindType_Hat && mappedKey.value == i)
					{
						if (usageCountHats[mappedKey.value] > 0)
							updatedInputState[buttonLabelList[mappedKey.keybindId]] = 1;
						else if (updatedInputState.find(buttonLabelList[mappedKey.keybindId]) == updatedInputState.end()) // Only reset value if it wasn't set by a button already
							updatedInputState[buttonLabelList[mappedKey.keybindId]] = 0;
					}
				}
			}

			for (auto& k : updatedInputState)
				currentInputStatePad[k.first] = k.second;

		Error:
			SAFE_FREE(pPreparsedData);
			SAFE_FREE(pButtonCaps);
			SAFE_FREE(pValueCaps);
		}

		static void ParseRawInputKB(PRAWINPUT pRawInput)
		{
			Mappings* mapping = nullptr;

			for (auto& it : mapVector)
			{
				if (!it.hidPath.compare(TEXT("Keyboard")))
				{
					mapping = &it;
					break;
				}
			}

			if (mapping == NULL)
				return;

			for (auto& mappedKey : mapping->mappings)
			{
				if (mappedKey.bindType == KeybindType_Keyboard && mappedKey.value == pRawInput->data.keyboard.VKey)
				{
					// Alternatively, keep a counter for how many keys are pressing the keybind at once
					if (pRawInput->data.keyboard.Flags & RI_KEY_BREAK)
						currentInputStateKeyboard[buttonLabelList[mappedKey.keybindId]] = 0;
					else
						currentInputStateKeyboard[buttonLabelList[mappedKey.keybindId]] = 1;
				}
			}
		}

		void RawInputPad::ParseRawInput(PRAWINPUT pRawInput)
		{
			if (pRawInput->header.dwType == RIM_TYPEKEYBOARD)
				ParseRawInputKB(pRawInput);
			else if (pRawInput->header.dwType == RIM_TYPEHID)
				ParseRawInputHID(pRawInput);
		}

		int RawInputPad::Open()
		{
			PHIDP_PREPARSED_DATA pPreparsedData = nullptr;
			HIDD_ATTRIBUTES attr;

			Close();

			mappings.clear();
			currentInputStateKeyboard.clear();
			currentInputStatePad.clear();
			currentInputStateAnalog.clear();
			LoadMappings(mDevType, mapVector);

			std::wstring selectedDevice;
			LoadSetting(Python2Device::TypeName(), mPort, APINAME, N_DEVICE, selectedDevice);
			LoadSetting(Python2Device::TypeName(), mPort, APINAME, N_WHEEL_PT, mAttemptPassthrough);

			memset(&mOLRead, 0, sizeof(OVERLAPPED));
			memset(&mOLWrite, 0, sizeof(OVERLAPPED));

			if (mAttemptPassthrough)
			{
				mUsbHandle = INVALID_HANDLE_VALUE;
				std::wstring path;

				UINT nDevices;
				UINT errorCode = GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST));
				if (errorCode == 0 || errorCode == ERROR_INSUFFICIENT_BUFFER)
				{
					std::vector<RAWINPUTDEVICELIST> newlist(nDevices);
					nDevices = GetRawInputDeviceList(&newlist[0], &nDevices, sizeof(RAWINPUTDEVICELIST));
					if (nDevices > 0)
					{
						for (std::vector<RAWINPUTDEVICELIST>::const_iterator it = newlist.begin(); it != newlist.end(); ++it)
						{
							TCHAR name[1024] = {0};
							UINT nameSize = 1024;
							GetRawInputDeviceInfo(it->hDevice, RIDI_DEVICENAME, name, &nameSize);

							std::wstring devName = name;
							std::transform(devName.begin(), devName.end(), devName.begin(), ::toupper);

							auto vid = devName.find(L"VID_0000");
							auto pid = devName.find(L"PID_7305");
							if (nameSize > 0 && vid != std::wstring::npos && pid != std::wstring::npos)
							{
								Console.WriteLn(L"Python 2 I/O device: %s", name);
								path = devName;
							}
						}
					}
				}

				mUsbHandle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

				if (mUsbHandle != INVALID_HANDLE_VALUE)
				{
					mOLRead.hEvent = CreateEvent(0, 0, 0, 0);
					mOLWrite.hEvent = CreateEvent(0, 0, 0, 0);

					HidD_GetAttributes(mUsbHandle, &(attr));

					const bool isPython2 = (attr.VendorID == 0x0000) && (attr.ProductID == 0x7305);
					if (!isPython2)
					{
						Console.Warning("USB: Not a Python 2 I/O device.\n");
						mDoPassthrough = 0;
						Close();
					}
					else if (!mWriterThreadIsRunning)
					{
						if (mWriterThread.joinable())
							mWriterThread.join();
						mWriterThread = std::thread(RawInputPad::WriterThread, this);
					}

					if (mDoPassthrough)
					{
						// for passthrough only
						HidD_GetPreparsedData(mUsbHandle, &pPreparsedData);
						HidP_GetCaps(pPreparsedData, &(mCaps));
						HidD_FreePreparsedData(pPreparsedData);

						if (!mReaderThreadIsRunning)
						{
							if (mReaderThread.joinable())
								mReaderThread.join();
							mReaderThread = std::thread(RawInputPad::ReaderThread, this);
						}
					}
				}
				else
					Console.Warning(L"USB: Could not open device '%s'.\nPassthrough will not work.\n", path.c_str());
			}

			shared::rawinput::RegisterCallback(this);
			return 0;
		}

		int RawInputPad::Close()
		{
			if (mUsbHandle != INVALID_HANDLE_VALUE)
			{
				Reset();
				Sleep(100); // give WriterThread some time to write out Reset() commands
				CloseHandle(mUsbHandle);
				CloseHandle(mOLRead.hEvent);
				CloseHandle(mOLWrite.hEvent);
			}

			shared::rawinput::UnregisterCallback(this);
			mUsbHandle = INVALID_HANDLE_VALUE;
			return 0;
		}

		bool RawInputPad::GetKeyState(LPWSTR keybind)
		{
			auto it = currentInputStateKeyboard.find(keybind);
			if (it != currentInputStateKeyboard.end() && it->second > 0)
				return true;

			it = currentInputStatePad.find(keybind);
			if (it != currentInputStatePad.end() && it->second > 0)
				return true;

			return false;
		}

		double RawInputPad::GetKeyStateAnalog(LPWSTR keybind)
		{
			const auto it = currentInputStateAnalog.find(keybind);
			if (it == currentInputStateAnalog.end())
				return 0;
			return it->second;
		}

		bool RawInputPad::IsKeybindAvailable(LPWSTR keybind)
		{
			return currentInputStateKeyboard.find(keybind) != currentInputStateKeyboard.end() || currentInputStatePad.find(keybind) != currentInputStatePad.end();
		}

		bool RawInputPad::IsAnalogKeybindAvailable(LPWSTR keybind)
		{
			return currentInputStateAnalog.find(keybind) != currentInputStateAnalog.end();
		}

// ---------
#include "python2-config-res.h"

		INT_PTR CALLBACK ConfigurePython2DlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam);
		int RawInputPad::Configure(int port, const char* dev_type, void* data)
		{
			Win32Handles* h = (Win32Handles*)data;
			INT_PTR res = RESULT_FAILED;
			if (shared::rawinput::Initialize(h->hWnd))
			{
				std::vector<std::wstring> devList;
				std::vector<std::wstring> devListGroups;

				wxFileName iniPath = EmuFolders::Settings.Combine(wxString("Python2.ini"));
				if (iniPath.FileExists())
				{
					std::unique_ptr<wxFileConfig> hini(OpenFileConfig(iniPath.GetFullPath()));
					IniLoader ini((wxConfigBase*)hini.get());

					wxString groupName;
					long groupIdx = 0;
					auto foundGroup = hini->GetFirstGroup(groupName, groupIdx);
					while (foundGroup)
					{
						if (groupName.StartsWith(L"GameEntry"))
							devListGroups.push_back(std::wstring(groupName));

						foundGroup = hini->GetNextGroup(groupName, groupIdx);
					}

					for (auto& groupName : devListGroups)
					{
						ScopedIniGroup groupEntry(ini, groupName);

						wxString tmp = wxEmptyString;
						ini.Entry(L"Name", tmp, wxEmptyString);
						if (tmp.empty())
							continue;

						devList.push_back(std::wstring(tmp));
					}
				}

				Python2DlgConfig config(port, dev_type, devList, devListGroups);
				res = DialogBoxParam(h->hInst, MAKEINTRESOURCE(IDD_PYTHON2CONFIG), h->hWnd, ConfigurePython2DlgProc, (LPARAM)&config);
				shared::rawinput::Uninitialize();
			}
			return (int)res;
		}

	} // namespace raw
} // namespace usb_pad
