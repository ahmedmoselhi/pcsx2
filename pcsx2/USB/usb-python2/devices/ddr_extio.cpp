#include "ddr_extio.h"

namespace usb_python2
{
	// I don't really know anything about this device but it seems to be related to lights
	void extio_device::write(std::vector<uint8_t>& packet)
	{
		if (!isOpen)
			return;

		/*
		* DDR:
		* 80 00 40 40 CCFL
		* 90 00 00 10 1P FOOT LEFT
		* c0 00 00 40 1P FOOT UP
		* 88 00 00 08 1P FOOT RIGHT
		* a0 00 00 20 1P FOOT DOWN
		* 80 10 00 10 2P FOOT LEFT
		* 80 40 00 40 2P FOOT UP
		* 80 08 00 08 2P FOOT RIGHT
		* 80 20 00 20 2P FOOT DOWN
		*/

		std::vector<uint8_t> response;

		while (packet.size() > 0)
		{
			if (packet[0] == 0xaa)
				response.push_back(0xaa);
			else if (packet[0] == 0x00)
				response.push_back(0x11);

			packet.erase(packet.begin(), packet.begin() + 1);
		}

		add_packet(response);
	}
} // namespace usb_python2