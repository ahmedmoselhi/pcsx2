#include "ddr_extio.h"

namespace usb_python2
{
	enum
	{
		EXTIO_LIGHT_PANEL_UP = 0x40,
		EXTIO_LIGHT_PANEL_DOWN = 0x20,
		EXTIO_LIGHT_PANEL_LEFT = 0x10,
		EXTIO_LIGHT_PANEL_RIGHT = 0x08,

		EXTIO_LIGHT_SENSOR_UP = 0x10,
		EXTIO_LIGHT_SENSOR_DOWN = 0x18,
		EXTIO_LIGHT_SENSOR_LEFT = 0x20,
		EXTIO_LIGHT_SENSOR_RIGHT = 0x28,
		EXTIO_LIGHT_SENSOR_ALL = 0x08,

		EXTIO_LIGHT_NEON = 0x40,
	};

	// Reference: https://github.com/nchowning/open-io/blob/master/extio-emulator.ino
	void extio_device::write(std::vector<uint8_t>& packet)
	{
		if (!isOpen)
			return;

		#if 0
		printf("EXTIO packet: ");
		for (int i = 0; i < packet.size(); i++)
			printf("%02x ", packet[i]);
		printf("\n");
		#endif

		if (packet.size() != 4)
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

		const auto expectedChecksum = packet[3];
		const uint8_t calculatedChecksum = (packet[0] + packet[1] + packet[2]) & 0x7f;

		if (calculatedChecksum != expectedChecksum)
		{
			printf("EXTIO packet checksum invalid! %02x vs %02x\n", expectedChecksum, calculatedChecksum);
			return;
		}
		
		#if 0
		const auto p1PanelLights = packet[0] & 0x7f;
		const auto p2PanelLights = packet[1] & 0x7f;
		const auto neonLights = packet[2];
		const auto panelSensors = packet[3] & 0x3f;

		printf("extio p1 up[%d] down[%d] left[%d] right[%d]\n", !!(p1PanelLights & EXTIO_LIGHT_PANEL_UP), !!(p1PanelLights & EXTIO_LIGHT_PANEL_DOWN), !!(p1PanelLights & EXTIO_LIGHT_PANEL_LEFT), !!(p1PanelLights & EXTIO_LIGHT_PANEL_RIGHT));
		printf("extio p2 up[%d] down[%d] left[%d] right[%d]\n", !!(p2PanelLights & EXTIO_LIGHT_PANEL_UP), !!(p2PanelLights & EXTIO_LIGHT_PANEL_DOWN), !!(p2PanelLights & EXTIO_LIGHT_PANEL_LEFT), !!(p2PanelLights & EXTIO_LIGHT_PANEL_RIGHT));
		printf("extio neons on[%d]\n", !!(neonLights & EXTIO_LIGHT_NEON));
		printf("extio sensors up[%d] down[%d] left[%d] right[%d] all[%d]\n", panelSensors == EXTIO_LIGHT_SENSOR_UP, panelSensors == EXTIO_LIGHT_SENSOR_DOWN, panelSensors == EXTIO_LIGHT_SENSOR_LEFT, panelSensors == EXTIO_LIGHT_SENSOR_RIGHT, panelSensors == EXTIO_LIGHT_SENSOR_ALL);
		printf("\n");
		#endif

		std::vector<uint8_t> response;
		response.push_back(0x11);
		packet.erase(packet.begin(), packet.begin() + 4);

		add_packet(response);
	}
} // namespace usb_python2