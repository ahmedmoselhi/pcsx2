#include "thrilldrive_handle.h"

namespace usb_python2
{
	bool thrilldrive_handle_device::device_write(std::vector<uint8_t>& packet, std::vector<uint8_t>& outputResponse)
	{
		const auto header = (ACIO_PACKET_HEADER*)packet.data();
		const auto code = BigEndian16(header->code);

		std::vector<uint8_t> response;
		bool isEmptyResponse = false;
		if (code == 0x0120)
		{
			uint8_t resp[3] = {0};

			// Big endian uint16_t
			// If resp[0] is non-0 then this value isn't read not read
			resp[0] = 0;
			resp[1] = 0;
			resp[2] = 0;

			response.insert(response.end(), std::begin(resp), std::end(resp));
		}
		else
		{
			// Just return 0 for anything else
			response.push_back(0);
		}

		if (response.size() > 0 || isEmptyResponse)
		{
			outputResponse.insert(outputResponse.end(), response.begin(), response.end());
			return true;
		}

		return false;
	}
} // namespace usb_python2