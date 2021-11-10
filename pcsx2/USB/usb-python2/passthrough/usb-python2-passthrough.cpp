#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "usb-python2-passthrough.h"

#ifdef _WIN32
#include "USB/Win32/Config_usb.h"
#include <wx/fileconf.h>
#include "common/IniInterface.h"
#include "gui/AppConfig.h"
#endif

#include <algorithm>

constexpr int USB_ENDPOINT_INTERRUPT = (LIBUSB_ENDPOINT_IN | 3);
constexpr int USB_ENDPOINT_IN = (LIBUSB_ENDPOINT_IN | 1);
constexpr int USB_ENDPOINT_OUT = (LIBUSB_ENDPOINT_OUT | 2);
constexpr int USB_TIMEOUT = 100;

namespace usb_python2
{
	namespace passthrough
	{
		void PassthroughInput::InterruptReaderThread(void* ptr)
		{
			PassthroughInput* dev = static_cast<PassthroughInput*>(ptr);
			dev->isInterruptReaderThreadRunning = true;

			int sinceLastUpdate = 0;

			while (dev->handle != NULL)
			{
				if (sinceLastUpdate >= 100)
				{
					// Just to make sure the last input update isn't too stale the next time it's ready, use a counter to force it to update again
					dev->isIoDataReady.store(false);
				}

				if (!dev->isIoDataReady.load())
				{
					auto ret = libusb_interrupt_transfer(dev->handle, USB_ENDPOINT_INTERRUPT, dev->ioData, sizeof(dev->ioData), NULL, 0);
					if (ret == 0)
						dev->isIoDataReady.store(true);

					sinceLastUpdate = 0;
				}
				else
				{
					sinceLastUpdate++;
				}
			}

			dev->isInterruptReaderThreadRunning = false;
		}

		int PassthroughInput::ReadPacket(std::vector<uint8_t>& data)
		{
			if (handle == NULL)
				return 0;

			uint8_t receiveBuf[64] = {0};
			int nread = 0;

			auto ret = libusb_bulk_transfer(handle, USB_ENDPOINT_IN, receiveBuf, sizeof(receiveBuf), &nread, 0);
			if (ret)
				return -1;

			//printf("Received %d bytes from device\n", nread);
			data.insert(data.end(), std::begin(receiveBuf), std::begin(receiveBuf) + nread);

			return nread;
		}

		int PassthroughInput::WritePacket(const std::vector<uint8_t>& data)
		{
			if (handle == NULL)
				return 0;

			return libusb_bulk_transfer(handle, USB_ENDPOINT_OUT, (unsigned char*)data.data(), data.size(), NULL, USB_TIMEOUT);
		}

		void PassthroughInput::ReadIo(std::vector<uint8_t> &data)
		{
			if (isIoDataReady.load())
			{
				memcpy(ioDataLastUpdate, ioData, sizeof(ioData));
				isIoDataReady.store(false);
			}

			data.insert(data.end(), std::begin(ioDataLastUpdate), std::begin(ioDataLastUpdate) + sizeof(ioDataLastUpdate));
		}

		int PassthroughInput::Open()
		{
			libusb_init(&ctx);

			handle = libusb_open_device_with_vid_pid(ctx, 0x0000, 0x7305);
			if (!handle) {
				printf("Could not open P2IO!\n");
				return 1;
			}

			const auto ret = libusb_claim_interface(handle, 0);
			if (ret < 0) {
				printf("Could not claim P2IO interface!\n");
				return 2;
			}

			printf("Opened P2IO device for passthrough!\n");

			memset(ioData, 0, sizeof(ioData));

			if (!isInterruptReaderThreadRunning)
			{
				if (interruptThread.joinable())
					interruptThread.join();
				interruptThread = std::thread(PassthroughInput::InterruptReaderThread, this);
			}

			return 0;
		}

		int PassthroughInput::Close()
		{
			if (handle != NULL)
				libusb_close(handle);

			handle = NULL;

			return 0;
		}

#ifdef _WIN32
// ---------
#include "win32/python2-config-passthrough-res.h"

		INT_PTR CALLBACK ConfigurePython2PassthroughDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam);
		int PassthroughInput::Configure(int port, const char* dev_type, void* data)
		{
			Win32Handles* h = (Win32Handles*)data;
			INT_PTR res = RESULT_FAILED;

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

				Python2DlgConfig config(port, dev_type, devList, devListGroups);
				res = DialogBoxParam(h->hInst, MAKEINTRESOURCE(IDD_PYTHON2CONFIGPASS), h->hWnd, ConfigurePython2PassthroughDlgProc, (LPARAM)&config);
			}
			return (int)res;
		}
#else
		int PassthroughInput::Configure(int port, const char* dev_type, void* data)
		{
			return 0;
		}
#endif

	} // namespace passthrough
} // namespace usb_python2
