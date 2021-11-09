#include "PrecompiledHeader.h"
#include "USB/USB.h"
#include "usb-python2-passthrough.h"

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

			while (dev->handle != NULL)
			{
				uint8_t receiveBuf[12] = {0};
				int nread = 0;

				auto ret = libusb_interrupt_transfer(dev->handle, USB_ENDPOINT_INTERRUPT, receiveBuf, sizeof(receiveBuf), &nread, 0);
				if (ret == 0)
				{
					if (!dev->isIoDataBusy.load())
						memcpy(dev->ioData, receiveBuf, std::min(nread, 12));
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
			isIoDataBusy.store(true);
			data.insert(data.end(), std::begin(ioData), std::begin(ioData) + sizeof(ioData));
			isIoDataBusy.store(false);
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
			{
				libusb_close(handle);
				libusb_exit(ctx);
			}

			handle = NULL;

			return 0;
		}
	} // namespace passthrough
} // namespace usb_python2
