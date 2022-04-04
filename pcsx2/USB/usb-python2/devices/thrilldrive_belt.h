#include "USB/usb-python2/usb-python2.h"
#include "acio.h"

namespace usb_python2
{
	class thrilldrive_belt_device : public acio_device_base
	{
	private:
		Python2Input* p2dev;
		bool seatBeltStatus = false;
		bool seatBeltButtonPressed = false;
		uint8_t seatBeltMotor1 = 0, seatBeltMotor2 = 0;

		void write(std::vector<uint8_t>& packet) {}

	public:
		thrilldrive_belt_device(Python2Input* device) noexcept
		{
			p2dev = device;
		}

		bool device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& outputResponse);
	};
} // namespace usb_python2
#pragma once
